/*
 *  Copyright (C) 2004-2009  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  Expression evaluator.
 *
 *
 *  TODO:
 *	Sign-extension is arch dependent (e.g. MIPS has it).
 *	SPECIAL IMPORTANT CASE: Clear the delay_slot flag when writing
 *		to the pc register.
 *	TAB completion? :-)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cpu.h"
#include "debugger.h"
#include "machine.h"
#include "misc.h"
#include "settings.h"


extern struct settings *global_settings;

extern int debugger_cur_cpu;
extern int debugger_cur_machine;


/*
 *  debugger_parse_name():
 *
 *  This function takes a string as input, and tries to match it to a register
 *  name or a more general "setting", a hexadecimal or decimal numeric value,
 *  or a registered symbol.
 *
 *  Some examples:
 *
 *	Settings (including register names):
 *		verbose
 *		pc
 *		r5
 *
 *	Numeric values:
 *		12345
 *		0x7fff1234
 *
 *	Symbols:
 *		memcpy
 *
 *  To force detection of different types, a character can be added in front of
 *  the name: "$" for numeric values, "#" for registers or other settings,
 *  and "@" for symbols.
 *
 *  Return value is:
 *
 *	PARSE_NOMATCH		no match
 *	PARSE_MULTIPLE		multiple matches
 *
 *  or one of these (and then *valuep is read or written, depending on
 *  the writeflag):
 *
 *	PARSE_SETTINGS		a setting (e.g. a register)
 *	PARSE_NUMBER		a hex number
 *	PARSE_SYMBOL		a symbol
 */
int debugger_parse_name(struct machine *m, char *name, int writeflag,
	uint64_t *valuep)
{
	int match_settings = 0, match_symbol = 0, match_numeric = 0;
	int skip_settings, skip_numeric, skip_symbol;

	if (m == NULL || name == NULL) {
		fprintf(stderr, "debugger_parse_name(): NULL ptr\n");
		exit(1);
	}

	while (name[0] == '\t' || name[0] == ' ')
		name ++;

	/*  Warn about non-signextended values:  */
	if (writeflag) {
		if (m->cpus[0]->is_32bit) {
			/*  Automagically sign-extend.  TODO: Is this good?  */
			if (((*valuep) >> 32) == 0 && (*valuep) & 0x80000000ULL)
				(*valuep) |= 0xffffffff00000000ULL;
		} else {
			if (((*valuep) >> 32) == 0 && (*valuep) & 0x80000000ULL)
				printf("WARNING: The value is not sign-extende"
				    "d. Is this what you intended?\n");
		}
	}

	skip_settings = name[0] == '$' || name[0] == '@';
	skip_numeric  = name[0] == '#' || name[0] == '@';
	skip_symbol   = name[0] == '$' || name[0] == '#';

	if (!skip_settings) {
		char setting_name[400];
		int res;

		res = settings_access(global_settings, name, writeflag, valuep);
		if (res == SETTINGS_OK)
			match_settings = 1;

		if (!match_settings) {
			snprintf(setting_name, sizeof(setting_name),
			    GLOBAL_SETTINGS_NAME".%s", name);
			res = settings_access(global_settings, setting_name,
			    writeflag, valuep);
			if (res == SETTINGS_OK)
				match_settings = 1;
		}

		if (!match_settings) {
			snprintf(setting_name, sizeof(setting_name),
			    GLOBAL_SETTINGS_NAME".emul.%s", name);
			res = settings_access(global_settings, setting_name,
			    writeflag, valuep);
			if (res == SETTINGS_OK)
				match_settings = 1;
		}

		if (!match_settings) {
			snprintf(setting_name, sizeof(setting_name),
			    GLOBAL_SETTINGS_NAME".emul.machine[%i].%s",
			    debugger_cur_machine, name);
			res = settings_access(global_settings, setting_name,
			    writeflag, valuep);
			if (res == SETTINGS_OK)
				match_settings = 1;
		}

		if (!match_settings) {
			snprintf(setting_name, sizeof(setting_name),
			    GLOBAL_SETTINGS_NAME".emul.machine[%i]."
			    "cpu[%i].%s", debugger_cur_machine,
			    debugger_cur_cpu, name);
			res = settings_access(global_settings, setting_name,
			    writeflag, valuep);
			if (res == SETTINGS_OK)
				match_settings = 1;
		}
	}

	/*  Check for a number match:  */
	if (!skip_numeric && isdigit((int)name[0])) {
		uint64_t x;
		x = strtoull(name, NULL, 0);
		if (writeflag)
			printf("You cannot assign like that.\n");
		else
			*valuep = x;
		match_numeric = 1;
	}

	/*  Check for a symbol match:  */
	if (!skip_symbol) {
		uint64_t newaddr;
		if (get_symbol_addr(&m->symbol_context, name, &newaddr)) {
			if (writeflag)
				printf("You cannot assign like that.\n");
			else
				*valuep = newaddr;
			match_symbol = 1;
		}
	}

	if (match_settings + match_symbol + match_numeric > 1)
		return PARSE_MULTIPLE;

	if (match_settings)
		return PARSE_SETTINGS;
	if (match_numeric)
		return PARSE_NUMBER;
	if (match_symbol)
		return PARSE_SYMBOL;

	return PARSE_NOMATCH;
}


/*
 *  debugger_parse_expression()
 *
 *  Input:
 *	writeflag = 0:	expr = an expression to evaluate. The result is
 *			returned in *valuep.
 *
 *	writeflag = 1:	expr = an lvalue name. *valuep is written to that
 *			lvalue, using debugger_parse_name().
 *
 *  Parentheses always have precedence.
 *  * / and % have second highest precedence.
 *  + - & | ^ have lowest precedence.
 *
 *  Return value on failure is:
 *
 *	PARSE_NOMATCH		one or more words in the expression didn't
 *				match any known symbol/register/number
 *	PARSE_MULTIPLE		multiple matches within the expression
 *
 *  Return value on success is PARSE_NUMBER (for now).
 *
 *
 *  TODO: BETTER RETURN VALUE!
 *
 *  NOTE: This is a quick hack, but hopefully it should work. The internal
 *        mechanism is to split the expression into a left half and a right
 *        half around an operator. This operator should be the operator
 *        in the string which has the lowest precedence (except those that
 *        are inside parentheses sub-expressions). E.g. if the expression
 *        is   a * (b + c * d) / e   then the operator with the lowest
 *        precedence is the first multiplication sign, and the split will
 *        be:   left  = a
 *              right = (b+c*d)/e
 */
int debugger_parse_expression(struct machine *m, char *expr, int writeflag,
	uint64_t *valuep)
{
	int prec, res, i, nest;
	char *copy;

	if (writeflag)
		return debugger_parse_name(m, expr, writeflag, valuep);

	while (expr[0] == '\t' || expr[0] == ' ')
		expr ++;

	CHECK_ALLOCATION(copy = strdup(expr));

	while (copy[0] && copy[strlen(copy)-1] == ' ')
		copy[strlen(copy)-1] = '\0';

	/*  Find the lowest operator precedence:  */
	i = 0; prec = 2; nest = 0;
	while (copy[i] != '\0') {
		switch (copy[i]) {
		case '(':
			nest ++;
			break;
		case ')':
			nest --;
			break;
		case '+':
		case '-':
		case '^':
		case '&':
		case '|':
			if (nest == 0)
				prec = 0;
			break;
		case '*':
		case '/':
		case '%':
			if (nest == 0 && prec > 1)
				prec = 1;
			break;
		}

		i++;
	}

	if (nest != 0) {
		printf("Unmatching parentheses.\n");
		return PARSE_NOMATCH;
	}

	if (prec == 2 && copy[0] == '(' && copy[strlen(copy)-1] == ')') {
		int res;
		copy[strlen(copy)-1] = '\0';
		res = debugger_parse_expression(m, copy+1, 0, valuep);
		free(copy);
		return res;
	}

	/*  Split according to the first lowest priority operator:  */
	i = 0; nest = 0;
	while (copy[i] != '\0') {
		switch (copy[i]) {
		case '(':
			nest ++;
			break;
		case ')':
			nest --;
			break;
		case '*':
		case '/':
		case '%':
			if (prec == 0)
				break;
			/*  Fallthrough.  */
		case '+':
		case '-':
		case '^':
		case '&':
		case '|':
			if (nest == 0) {
				uint64_t left, right;
				int res1, res2, j;
				char op = copy[i];

				copy[i] = '\0';
				j = i;
				while (j>0 && copy[j-1] == ' ') {
					copy[j-1] = '\0';
					j --;
				}

				res1 = debugger_parse_expression(
				    m, copy, 0, &left);
				res2 = debugger_parse_expression(
				    m, copy + i + 1, 0, &right);

				if (res1 == PARSE_NOMATCH ||
				    res2 == PARSE_NOMATCH) {
					res = PARSE_NOMATCH;
					goto return_failure;
				}

				if (res1 == PARSE_MULTIPLE ||
				    res2 == PARSE_MULTIPLE) {
					res = PARSE_MULTIPLE;
					goto return_failure;
				}

				switch (op) {
				case '+':
					(*valuep) = left + right;
					break;
				case '-':
					(*valuep) = left - right;
					break;
				case '^':
					(*valuep) = left ^ right;
					break;
				case '&':
					(*valuep) = left & right;
					break;
				case '|':
					(*valuep) = left | right;
					break;
				case '*':
					(*valuep) = left * right;
					break;
				case '/':
					(*valuep) = left / right;
					break;
				case '%':
					(*valuep) = left % right;
					break;
				}

				goto return_ok;
			}
			break;
		}

		i ++;
	}

	res = debugger_parse_name(m, expr, writeflag, valuep);
	if (res == PARSE_NOMATCH || res == PARSE_MULTIPLE)
		goto return_failure;

return_ok:
	free(copy);
	return PARSE_NUMBER;

return_failure:
	free(copy);
	return res;
}

