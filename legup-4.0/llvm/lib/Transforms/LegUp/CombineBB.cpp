/*===- CombineBB.cpp - LegUp uses this pass to combine basicblocks --===//
 *
 * This file is distributed under the LegUp license. See LICENSE for details.
 *
 *===----------------------------------------------------------------------===//
 * Author: Joy (Yu Ting) Chen
 * Last Revision: Apr 10/14
 *
 * This pass prints out a list of all BB indexes which form a collapsable structure
 * Type A:
 *            + (BBH)
 *           /|
 *    (BBM) + |
 *           \|
 *            + (BBT)
 * Type B:
 *            + (BBH)
 *           / \
 *    (BBL) +   + (BBR)
 *           \ /
 *            + (BBT)
 *
 *===----------------------------------------------------------------------===*/

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Use.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/StringExtras.h"
#include "../LegUp/utils.h"
#include "llvm/IR/CFG.h" 
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/LoopInfo.h"
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <map>
#include <string>

#ifndef PROFILE_COMBINE
#define PROFILE_COMBINE
#endif

#define THRESH_A	1
#define THRESH_B	10
#define PRED_STORE	1

// The pass will run through all functions, and all basic blocks.
// For each basic block, if the BB has exactly two parents, it will search
// each parent to see if each parent has only one parent. If the BB's two parents
// each have only one parent and they are a match, then a structure has been found

using namespace llvm;

static cl::opt<bool>
ProfileOutput("profile-output", cl::init(false),
	cl::desc("Output bitcode for profiling, does not purposefully reference null address"));

static cl::opt<int>
COMBINE_BB("combine-bb", cl::init(0),
	cl::desc("Combine Basic Block, 0 - no combine, 1 - combine all, 2 - selectively combine"));

static cl::opt<bool>
LOOPS_ONLY("loops-only", cl::init(false)),
	cl::desc("Only combine patterns within loops");

namespace legup
{
	typedef std::pair<std::string, std::string> StrPair;

	struct CombineList {
		char type;
		bool overlap; // true if head of this pattern also belongs to tail of previous pattern, else false

		union {
			struct {
				BasicBlock *head;
				BasicBlock *middle;
				BasicBlock *tail;
				// block cycle info
				unsigned int hcycs;
				unsigned int mcycs;
				unsigned int tcycs;
				// block repetition info
				unsigned int hreps;
				unsigned int mreps;
				unsigned int treps;
			} pA;

			struct {
				BasicBlock *head;
				BasicBlock *left; // 'true' branch target
				BasicBlock *right; // 'false' branch target
				BasicBlock *tail;
				// block cycle info
				unsigned int hcycs;
				unsigned int lcycs;
				unsigned int rcycs;
				unsigned int tcycs;
				// block repetition info
				unsigned int hreps;
				unsigned int lreps;
				unsigned int rreps;
				unsigned int treps;
			} pB;
		} U;

		CombineList() {}
	};

    class LegUpCombineBB : public FunctionPass
    {
		public:
        static char ID;             // Pass identification, replacement for typeid
        LegUpCombineBB() : FunctionPass(ID) {};
		virtual bool runOnFunction(Function &F);

		private:
		void runCombine0(Function &F);
		void runCombine1(Function &F);
		void runCombine2(Function &F);
		void initializeCombineBB();
		void gen_cyc_hash(std::map <StrPair, int> &cyc_hash, FILE *fp);
		void gen_rep_hash(std::map <StrPair, int> &rep_hash, FILE *fp);
		void construct_basicblock_map_from_file(FILE *fp);
		void print_my_hash(std::map<StrPair, int> &hash);
		void print_basicblock_hash_to_file();
		int remove_degraded_merges(Function &F, std::vector<CombineList> &list);
		bool compare_merge_cost_patternA(Function &F, CombineList p);
		bool compare_merge_cost_patternB(Function &F, CombineList p);
		bool predict_merge_cost(CombineList p);
		void find_combine_patterns(Function &F, std::vector<CombineList> &clist, int combine);
		BasicBlock *is_patternA_tail(BasicBlock *B, LoopInfo &LoopInfoObj);
		bool is_patternA(BasicBlock *BBH, BasicBlock *BBM, BasicBlock *BBT);
		BasicBlock *is_patternB_tail(BasicBlock *B, LoopInfo &LoopInfoObj);
		bool is_patternB(BasicBlock *BBH, BasicBlock *BBL, BasicBlock *BBR, BasicBlock *BBT);
		void add_pattern_to_list(Function &F, std::vector<CombineList> &list, BasicBlock *H, BasicBlock *B, char type, int combine);
		void update_basicblock_hash_patternA(Function &F, std::vector<CombineList> &list, CombineList &node, int &num_merged_blks, int &num_merged_patterns);
		void update_basicblock_hash_patternB(Function &F, std::vector<CombineList> &list, CombineList &node, int &num_merged_blks, int &num_merged_patterns);
		bool head_belongs_to_previous_pattern(Function &F, std::vector<CombineList> &list, BasicBlock *head);
		void propagate_label_to_previous_patterns(Function &F, std::vector<CombineList> &list, BasicBlock *head);
		bool is_number (std::string &str);
		void print_CombineList(std::vector<CombineList> &list);
		void debug_print_CombineList(std::vector<CombineList> &list, int v);
		void merge_patterns_in_list(Function &F, std::vector<CombineList> &list);
		bool merge_basicblock_patternA(Function &F, BasicBlock *BBH, BasicBlock *BBM, BasicBlock *BBT);
		bool merge_basicblock_patternB(Function &F, BasicBlock *BBH, BasicBlock *BBL, BasicBlock *BBR, BasicBlock *BBT);
		void move_instructions_and_insert_PHI(BasicBlock *BBL, BasicBlock *BBR, BasicBlock *BBT, Value *cond, int condflag);
		void merge_basicblock_with_store(Function &F, BasicBlock *BB, Value *cond, int condflag);
		void remove_last_branch_instruction (BasicBlock *BB);
		bool BasicBlockMayWriteToMemory(BasicBlock *BB);
		bool BasicBlockContainsCallInst(BasicBlock *BB);
		// Basic Block cycle mapping
		std::map <StrPair, int> cyc_hash;
		std::map <StrPair, int> rep_hash;
		std::map <StrPair, int> mod_cyc_hash;
		std::map <StrPair, int> mod_rep_hash;
		
		// Basic Block map for exact cycle prediction of new merged block
		// {function, old_label}->{function, new_label}, although function should not change...
		std::map<StrPair, StrPair> basicblock_hash;
		int num_total_blks;
		int num_merged_blks;
		int num_merged_patterns;

		int allocaNum;

		bool loops_only;

		virtual void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.addRequired<LoopInfo>();
		}

		int combine;
		bool profile;
	};// class LegUpCombineBB

		void LegUpCombineBB::initializeCombineBB() {
			// resetting all hashes and variables
			cyc_hash.clear();
			rep_hash.clear();
			mod_cyc_hash.clear();
			mod_rep_hash.clear();
			basicblock_hash.clear();
			num_total_blks = 0;
			num_merged_blks = 0;
			num_merged_patterns = 0;
		}

        bool LegUpCombineBB::runOnFunction(Function &F) {
			#ifdef COMBINE_DEBUG
			errs() << "runOnFunction " << F.getName() << " legup-combine-bb pass\n";
			#endif

			// set PROFILE_BB 1 when generating for profiling purposes, else set to 0
			//int combine = atoi(getenv("COMBINE_BB"));
			combine = COMBINE_BB;
			profile = ProfileOutput;
			loops_only = LOOPS_ONLY;
			#ifdef COMBINE_DEBUG
			errs() << "Variable COMBINE_BB = " << combine << "\n";
			errs() << "RUN PROFILE_BB = " << profile << "\n";
			#endif

			/* COMBINE_BB:
			 * value:
			 *	0 - no legup-combine-bb pass applied (i.e. this pass does not run)
			 *	1 - during first run, merge is performed at every opportunity and basic block
			 *		mapping is created > file
			 *	2 - second run after profiling info, can make a smarter decision. -- disabled for now
			 */

			switch (combine) {
				case 0:
					runCombine0(F);
					break;
				case 1:
					runCombine1(F);
					break;
				case 2:
					runCombine2(F);
					break;
				default:
					break;
			}
/*
			int discard = 0;
			allocaNum = 0;
		
			char name[50];
			if (combine == 2) {
				// open scheduling.legup.rpt
				FILE *osfp = fopen("scheduling.legup.precomb.rpt", "r");
				assert(osfp != NULL);
	
				// open Makefile to get $(NAME)
				FILE *mfp = fopen("Makefile", "r");
				assert(mfp != NULL);
				while (fscanf(mfp, "NAME=%s", name) != 1) ;
				std::string fname(name);
				fname = fname + ".precomb.lli_bb.trace";
				char *mfile = new char[fname.size() + 1];
				mfile[fname.size()] = 0;
				memcpy(mfile, fname.c_str(), fname.size());
				fclose(mfp);
	
				// open trace file
				FILE *otfp = fopen(mfile, "r");
				assert(otfp != NULL);
				
				// generate hash for original IR
				gen_cyc_hash(cyc_hash, osfp);
				gen_rep_hash(rep_hash, otfp);
	
				// close files
				fclose(osfp);
				fclose(otfp);
			}

			#ifdef COMBINE_DEBUG
				errs() << "cyc_hash printout\n";
				print_my_hash(cyc_hash);
				errs() << "rep_hash printout\n";
				print_my_hash(rep_hash);
			#endif

			if (combine == 2) {
				// open scheduling.legup.rpt
				FILE *nsfp = fopen("scheduling.legup.comb.rpt", "r");
				assert(nsfp != NULL);

				// open new trace file
				std::string nfname(name);
				nfname = nfname + ".comb.lli_bb.trace";
				char *nmfile = new char[nfname.size() + 1];
				nmfile[nfname.size()] = 0;
				memcpy(nmfile, nfname.c_str(), nfname.size());
				//if (debug > 1) errs() << "Trying to open file ntfp: " << nfname << "\n";
				FILE *ntfp = fopen(nmfile, "r");
				assert(ntfp != NULL);

				// generate hash for modified IR
				gen_cyc_hash(mod_cyc_hash, nsfp);
				gen_rep_hash(mod_rep_hash, ntfp);

				// close files
				fclose(nsfp);
				fclose(ntfp);

				#ifdef COMBINE_DEBUG
					errs() << "DEBUG output: mod_cyc_hash printout\n";
					print_my_hash(mod_cyc_hash);
					errs() << "DEBUG output: mod_rep_hash printout\n";
					print_my_hash(mod_rep_hash);
				#endif
			}

			// find list of combinable patterns
			std::vector<CombineList> clist;
			assert(clist.size() == 0);

			//errs() << "Before find_combine_patterns\n";
			find_combine_patterns(F, clist, combine);
			//errs() << "After find_combine_patterns\n";

			if (clist.size() == 0) {
			#ifdef COMBINE_DEBUG
				errs() << "======================================\n";
				errs() << "legup-combine-bb " << F.getName() << " : No patterns found\n";
				errs() << "======================================\n";
			#endif
				if (combine == 1 && !ProfileOutput) {
					FILE *pfp = fopen("combine_profile.rpt", "a");
					assert(pfp != NULL);
					fprintf(pfp, "(combine == %d) Function: %s - Number of BasicBlocks: %d Number of BasicBlocks found to be contained in a pattern structure that is mergeable (no mem): %d\n", combine, (F.getName()).str().c_str(), num_total_blks, num_merged_blks);
					fclose(pfp);
				}
				return true;
			}

		#ifdef COMBINE_DEBUG
			debug_print_CombineList(clist, 0);
		#endif
			
			if (combine == 1) {
				print_basicblock_hash_to_file();
			} else if (combine == 2) {
				FILE *bfp = fopen("basicblockmap.rpt", "r");
				assert(bfp != NULL);
				construct_basicblock_map_from_file(bfp);
				fclose(bfp);

				// remove those which are predicted to perform worse after merge
				discard = remove_degraded_merges(F, clist);
				#ifdef COMBINE_DEBUG
					errs() << "After remove_degraded_merges()\n";
					debug_print_CombineList(clist, 1);
				#endif
			}

			// print to file exactly the basic blocks to be merged
			print_CombineList(clist);

		#ifdef COMBINE_DEBUG
			errs() << "======================================\n";
			errs() << "Total number of patterns found: " << clist.size() << "\n";
			//if (combine == 2) {
			errs() << "Number of patterns discarded: " << discard << "\n";
			//}
			errs() << "======================================\n";
		#endif

			if (combine > 0) {
				// begin basic block merges
				merge_patterns_in_list(F, clist);
			}

			if (combine == 1 && !ProfileOutput) {
				FILE *pfp = fopen("combine_profile.rpt", "a");
				assert(pfp != NULL);
				fprintf(pfp, "(combine == %d) Function: %s - Number of BasicBlocks: %d Number of BasicBlocks found to be contained in a pattern structure that is mergeable (no mem): %d\n", combine, (F.getName()).str().c_str(), num_total_blks, num_merged_blks);
				fclose(pfp);
			}
			*/

			return true;
		}

		/* runCombine0()
		 * for combine == 0 mode
		 */
		void LegUpCombineBB::runCombine0(Function &F) {
		
			// find list of combinable patterns
			std::vector<CombineList> clist;
			assert(clist.size() == 0);

			//errs() << "Before find_combine_patterns\n";
			find_combine_patterns(F, clist, combine);
			//errs() << "After find_combine_patterns\n";

			if (clist.size() == 0) {
				#ifdef COMBINE_DEBUG
				errs() << "======================================\n";
				errs() << "legup-combine-bb " << F.getName() << " : No patterns found\n";
				errs() << "======================================\n";
				#endif
				return;
			}

			#ifdef COMBINE_DEBUG
			debug_print_CombineList(clist, 0);
			#endif
			
			// print to file exactly the basic blocks to be merged
			print_CombineList(clist);

			#ifdef COMBINE_DEBUG
			errs() << "======================================\n";
			errs() << "Total number of patterns found: " << clist.size() << "\n";
			errs() << "======================================\n";
			#endif

			return;
		}


		/* runCombine1()
		 * for combine == 1 mode
		 */
		void LegUpCombineBB::runCombine1(Function &F) {
		
			allocaNum = 0;
		
			// find list of combinable patterns
			std::vector<CombineList> clist;
			assert(clist.size() == 0);

			//errs() << "Before find_combine_patterns\n";
			find_combine_patterns(F, clist, combine);
			//errs() << "After find_combine_patterns\n";

			if (clist.size() == 0) {
				#ifdef COMBINE_DEBUG
				errs() << "======================================\n";
				errs() << "legup-combine-bb " << F.getName() << " : No patterns found\n";
				errs() << "======================================\n";
				#endif
				if (combine == 1 && !ProfileOutput) {
					FILE *pfp = fopen("combine_profile.rpt", "a");
					assert(pfp != NULL);
					fprintf(pfp, "(combine == %d) Function: %s - Number of BasicBlocks: %d Number of BasicBlocks found to be contained in a pattern structure that is mergeable (no mem): %d\n", combine, (F.getName()).str().c_str(), num_total_blks, num_merged_blks);
					fclose(pfp);
				}
				return;
			}

			#ifdef COMBINE_DEBUG
			debug_print_CombineList(clist, 0);
			#endif
			
			print_basicblock_hash_to_file();

			// print to file exactly the basic blocks to be merged
			print_CombineList(clist);

			#ifdef COMBINE_DEBUG
			errs() << "======================================\n";
			errs() << "Total number of patterns found: " << clist.size() << "\n";
			errs() << "======================================\n";
			#endif

			// begin basic block merges
			merge_patterns_in_list(F, clist);

			if (!ProfileOutput) {
				FILE *pfp = fopen("combine_profile.rpt", "a");
				assert(pfp != NULL);
				fprintf(pfp, "(combine == %d) Function: %s - Number of BasicBlocks: %d Number of BasicBlocks found to be contained in a pattern structure that is mergeable (no mem): %d\n", combine, (F.getName()).str().c_str(), num_total_blks, num_merged_blks);
				fclose(pfp);
			}

			return;
		}



		/* runCombine2()
		 * for combine == 2 mode
		 */
		void LegUpCombineBB::runCombine2(Function &F) {
			allocaNum = 0;
		
			char name[50];
			// open scheduling.legup.rpt
			FILE *osfp = fopen("scheduling.legup.precomb.rpt", "r");
			assert(osfp != NULL);
	
			// open Makefile to get $(NAME)
			FILE *mfp = fopen("Makefile", "r");
			assert(mfp != NULL);
			while (fscanf(mfp, "NAME=%s", name) != 1) ;
			std::string fname(name);
			fname = fname + ".precomb.lli_bb.trace";
			char *mfile = new char[fname.size() + 1];
			mfile[fname.size()] = 0;
			memcpy(mfile, fname.c_str(), fname.size());
			fclose(mfp);
	
			// open trace file
			FILE *otfp = fopen(mfile, "r");
			assert(otfp != NULL);
				
			// generate hash for original IR
			gen_cyc_hash(cyc_hash, osfp);
			gen_rep_hash(rep_hash, otfp);
	
			// close files
			fclose(osfp);
			fclose(otfp);

			#ifdef COMBINE_DEBUG
			errs() << "cyc_hash printout\n";
			print_my_hash(cyc_hash);
			errs() << "rep_hash printout\n";
			print_my_hash(rep_hash);
			#endif

			// open scheduling.legup.rpt
			FILE *nsfp = fopen("scheduling.legup.comb.rpt", "r");
			assert(nsfp != NULL);

			// open new trace file
			std::string nfname(name);
			nfname = nfname + ".comb.lli_bb.trace";
			char *nmfile = new char[nfname.size() + 1];
			nmfile[nfname.size()] = 0;
			memcpy(nmfile, nfname.c_str(), nfname.size());
			//if (debug > 1) errs() << "Trying to open file ntfp: " << nfname << "\n";
			FILE *ntfp = fopen(nmfile, "r");
			assert(ntfp != NULL);

			// generate hash for modified IR
			gen_cyc_hash(mod_cyc_hash, nsfp);
			gen_rep_hash(mod_rep_hash, ntfp);

			// close files
			fclose(nsfp);
			fclose(ntfp);

			#ifdef COMBINE_DEBUG
			errs() << "DEBUG output: mod_cyc_hash printout\n";
			print_my_hash(mod_cyc_hash);
			errs() << "DEBUG output: mod_rep_hash printout\n";
			print_my_hash(mod_rep_hash);
			#endif

			// find list of combinable patterns
			std::vector<CombineList> clist;
			assert(clist.size() == 0);

			//errs() << "Before find_combine_patterns\n";
			find_combine_patterns(F, clist, combine);
			//errs() << "After find_combine_patterns\n";

			if (clist.size() == 0) {
				#ifdef COMBINE_DEBUG
				errs() << "======================================\n";
				errs() << "legup-combine-bb " << F.getName() << " : No patterns found\n";
				errs() << "======================================\n";
				#endif
				return;
			}

			#ifdef COMBINE_DEBUG
			debug_print_CombineList(clist, 0);
			#endif
			
			FILE *bfp = fopen("basicblockmap.rpt", "r");
			assert(bfp != NULL);
			construct_basicblock_map_from_file(bfp);
			fclose(bfp);

			// remove those which are predicted to perform worse after merge
			#ifdef COMBINE_DEBUG
			int discard = 0;
			discard = remove_degraded_merges(F, clist);
			errs() << "After remove_degraded_merges()\n";
			debug_print_CombineList(clist, 1);
			#endif

			// print to file exactly the basic blocks to be merged
			print_CombineList(clist);

			#ifdef COMBINE_DEBUG
			errs() << "======================================\n";
			errs() << "Total number of patterns found: " << clist.size() << "\n";
			errs() << "Number of patterns discarded: " << discard << "\n";
			errs() << "======================================\n";
			#endif

			// begin basic block merges
			merge_patterns_in_list(F, clist);

			return;
		}

		/* generate a hash of the basic blocks and the number of cycles they require
		 * mapping: {key} -> val
		 *			{function, label} -> cycles
		 */
		void LegUpCombineBB::gen_cyc_hash(std::map <StrPair, int> &cyc_hash, FILE *fp) {
			assert(fp != NULL);
			char line[200], matchb[14], matchf[17], fnc[100], lbl[100];
			int cycle;
			// parse file to get basic block cycle information
			while (fgets(line, 200, fp)) { // 200 characters should be enough...
				// get function name
				strncpy(matchf, line, 16);
				matchf[16] = '\0'; // manually add null
				if (strcmp(matchf, "Start Function: ") == 0) {
					int res = sscanf(line, "Start Function: %s", fnc);
					if (res != 1) {
						// this shouldn't happen
						assert(0);
					}
				}

				// get basic block cycle info
				strncpy(matchb, line, 13);
				matchb[13] = '\0'; //manually add null
				if (strcmp(matchb, "Basic Block: ") == 0) {
					int res = sscanf(line, "Basic Block: %s Num States: %d", lbl, &cycle);
					if (res == 2) {
						// add {label} -> cycle
						// lbl is a null-terminating char array
						// fnc is a null-terminating char array
						std::string label(lbl);
						std::string function(fnc);
						cyc_hash[std::make_pair(function, label)] = cycle;
					}
				}
			}
			return;
		}


		/* generate a hash of the basic blocks and the number of times they repeat
		 * mapping: {key} -> val
		 *			{function, label} -> reps
		 */
		void LegUpCombineBB::gen_rep_hash(std::map <StrPair, int> &rep_hash, FILE *fp) {
			assert(fp != NULL);
			char line[200], fnc[100], lbl[100];
			// parse file to get basic block repetition information 
			// initialize map to 0 
			while (fgets(line, 200, fp)) {
				int res = sscanf(line,  "<%[^>]>:<%[^>]>", fnc, lbl);
				if (res == 2) {
					// initialize map to 0
					std::string label(lbl);
					std::string function(fnc);
					rep_hash[std::make_pair(function, label)] = 0;
				}
			}

			// restart at beginning of file
			rewind(fp);

			while (fgets(line, 200, fp)) {
				int res = sscanf(line, "<%[^>]>:<%[^>]>", fnc, lbl);
				if (res == 2) {
					std::string function(fnc);
					std::string label(lbl);
					rep_hash[std::make_pair(function, label)] = rep_hash[std::make_pair(function, label)] + 1;
				}
			}
			return;
		}


		/*
		 * Given a pointer to the file basicblockmap.rpt, regenerate the basic block hash
		 */
		void LegUpCombineBB::construct_basicblock_map_from_file(FILE *fp)
		{
			assert(fp != NULL);

			char line[400], ofnc[100], olbl[100], nfnc[100], nlbl[100];

			while (fgets(line, 400, fp)) { // 400 chars should be enough...
				int res = sscanf(line, "<%[^>]>:<%[^>]> maps to <%[^>]>:<%[^>]>", ofnc, olbl, nfnc, nlbl);
				if (res == 4) {
					std::string ofunction(ofnc);
					std::string olabel(olbl);
					std::string nfunction(nfnc);
					std::string nlabel(nlbl);
					basicblock_hash.insert(std::pair<StrPair, StrPair>(std::make_pair(ofunction, olabel), std::make_pair(nfunction, nlabel)));
				}
			}
			return;
		}


		/*
		 * print the cycles hash
		 */
		void LegUpCombineBB::print_my_hash(std::map<StrPair, int> &hash)
		{
			std::map<StrPair, int>::iterator it;
			for (it = hash.begin(); it != hash.end(); ++it) {
				errs() << "{" << it->first.first << "}{" << it->first.second << "} " << it->second << "\n";
			}
			return;
		}


		void LegUpCombineBB::print_basicblock_hash_to_file() {
			// print basicblock_hash to file
			FILE *bfp = fopen("basicblockmap.rpt", "a");
			assert(bfp != NULL);
			std::map <StrPair, StrPair>::iterator it;
			for (it = basicblock_hash.begin(); it != basicblock_hash.end(); ++it) {
				char f1[100] = {0};
				char l1[100] = {0};
				char f2[100] = {0};
				char l2[100] = {0};
				memcpy(f1, it->first.first.c_str(), it->first.first.size());
				memcpy(l1, it->first.second.c_str(), it->first.second.size());
				memcpy(f2, it->second.first.c_str(), it->second.first.size());
				memcpy(l2, it->second.second.c_str(), it->second.second.size());
				fprintf(bfp, "<%s>:<%s> maps to <%s>:<%s>\n", f1, l1, f2, l2);
			}
			fclose(bfp);
		}


		/* remove_degraded_merges()
		 * remove from list the patterns which will result in degraded performance
		 * keeping only the merges that are expected to produce performance gain
		 */
		int LegUpCombineBB::remove_degraded_merges(Function &F, std::vector<CombineList> &list) {
			if (list.size() == 0) {
				// no patterns present on list
				return 0;
			}

			int discard = 0;
			
			for (std::vector<CombineList>::iterator p = list.begin(); p != list.end(); ) {
				// if (! predict_merge_cost(*p)) { // predicted cost
				if (p->type == 'A') {
					if (! compare_merge_cost_patternA(F, *p)) { // exact cost
						// remove iterator and return new iterator
						p = list.erase(p);
						discard++;
						if (p == list.end()) {
							break;
						}
					} else {
						p++;
					}
				} else if (p->type == 'B') {
					if (! compare_merge_cost_patternB(F, *p)) { // exact cost
						// remove iterator and return new iterator
						p = list.erase(p);
						discard++;
						if (p == list.end()) {
							break;
						}
					} else {
						p++;
					}
				}
			}
			return discard;
		}


		/* compare_merge_cost_patternA()
		 * estimates and compares the required clock cycles of the unmodified and merged
		 * pattern pointed to by p
		 * return true if the merge is expected to reduce overall clock cycles
		 * return false otherwise
		 */
		bool LegUpCombineBB::compare_merge_cost_patternA(Function &F, CombineList p)
		{
			std::string funcName = F.getName();
			float oval = 0, nval = 0;
			int val1, val2;
			StrPair pair1, pair2;
			std::map <StrPair, StrPair>::iterator it;
			FILE *pfp = fopen("combine_profile.rpt", "a");
			// find all blocks merged within this pattern
			pair2 = basicblock_hash.find(std::make_pair(funcName, getLabel(p.U.pA.head)))->second;
			val1 = (mod_cyc_hash.find(pair2))->second;
			val2 = (mod_rep_hash.find(pair2))->second;
			nval = val1 * val2;
			if (!ProfileOutput) {
				fprintf(pfp, "Examining pattern A: %s-%s-%s\n", getLabel(p.U.pA.head).c_str(), getLabel(p.U.pA.middle).c_str(), getLabel(p.U.pA.tail).c_str());
				fprintf(pfp, "The blocks in this pattern map to %s\n", pair2.second.c_str());
				fprintf(pfp, "The new block has cyc %d, it %d, total cyc = %f\n", val1, val2, nval);
				fprintf(pfp, "The blocks that map to the new block are:\n");
			}
			for (it = basicblock_hash.begin(); it != basicblock_hash.end(); ++it) {
				if (std::make_pair((it->second).first, (it->second).second) == basicblock_hash[std::make_pair(funcName, getLabel(p.U.pA.head))]) {
					oval += (cyc_hash[std::make_pair((it->first).first, (it->first).second)])*(rep_hash[std::make_pair((it->first).first, (it->first).second)]);
					if (!ProfileOutput) {
						fprintf(pfp, "%s - cyc %d, it %d, oval increment to %f\n", it->first.second.c_str(), cyc_hash[it->first], rep_hash[it->first], oval);
					}
				}
			}
			if (nval <= oval) {
				#ifdef COMBINE_DEBUG
				errs() << nval << " <= " << oval << " DO NOT DISCARD A\n";
				#endif
				fclose (pfp);
				return true;
			}
			#ifdef COMBINE_DEBUG
			errs() << nval << " > " << oval << " DISCARD A\n";
			#endif
			// for profiling
			assert(pfp != NULL);
			if (!ProfileOutput) {
				fprintf(pfp, "Pattern A structure removed based on profiling\n");
			}
			fclose (pfp);
			return false;
		}

				
		/* compare_merge_cost_patternB()
		 * estimates and compares the required clock cycles of the unmodified and merged
		 * pattern pointed to by p
		 * return true if the merge is expected to reduce overall clock cycles
		 * return false otherwise
		 */
		bool LegUpCombineBB::compare_merge_cost_patternB(Function &F, CombineList p)
		{
			std::string funcName = F.getName();
			float oval = 0, nval = 0;
			int val1, val2;
			StrPair pair1, pair2;
			std::map <StrPair, StrPair>::iterator it;
			FILE *pfp = fopen("combine_profile.rpt", "a");
			pair2 = basicblock_hash.find(std::make_pair(funcName, getLabel(p.U.pB.head)))->second;

			val1 = (mod_cyc_hash.find(pair2))->second;
			val2 = (mod_rep_hash.find(pair2))->second;
			nval = val1 * val2;
			if (!ProfileOutput) {
				fprintf(pfp, "Examining pattern B: %s-%s-%s-%s\n", getLabel(p.U.pB.head).c_str(), getLabel(p.U.pB.left).c_str(), getLabel(p.U.pB.right).c_str(), getLabel(p.U.pB.tail).c_str());
				fprintf(pfp, "The blocks in this pattern map to %s\n", pair2.second.c_str());
				fprintf(pfp, "The new block has cyc %d, it %d, total cyc = %f\n", val1, val2, nval);
				fprintf(pfp, "The blocks that map to the new block are:\n");
			}
			for (it = basicblock_hash.begin(); it != basicblock_hash.end(); ++it) {
				if (it->second == basicblock_hash[std::make_pair(funcName, getLabel(p.U.pB.head))]) {
					oval += (cyc_hash[it->first])*(rep_hash[it->first]);
					if (!ProfileOutput) {
						fprintf(pfp, "%s - cyc %d, it %d, oval increment to %f\n", it->first.second.c_str(), cyc_hash[it->first], rep_hash[it->first], oval);
					}
				} else {
				}
			}

			if (nval <= oval) {
				#ifdef COMBINE_DEBUG
				errs() << nval << " <= " << oval << " DO NOT DISCARD\n";
				#endif
				fclose (pfp);
				return true;
			}
			#ifdef COMBINE_DEBUG
			errs() << nval << " > " << oval << " DISCARD\n";
			#endif

			// for profiling
			assert(pfp != NULL);
			if (!ProfileOutput) {
				fprintf(pfp, "Pattern B structure removed based on profiling\n");
			}
			fclose (pfp);
			return false;
		}

		
		/* predict_merge_cost()
		 * estimates and compares the required clock cycles of the unmodified and merged
		 * pattern pointed to by p
		 * return true if the merge is expected to reduce overall clock cycles
		 * return false otherwise
		 */
		bool LegUpCombineBB::predict_merge_cost(CombineList p) {
			float oval, nval;
			FILE *pfp = fopen("combine_profile.rpt", "a");
			switch (p.type) {
				case 'A':
					// keep this patternA if middle block executes most of the time
					assert(p.U.pA.hcycs != 0 && p.U.pA.mcycs != 0 && p.U.pA.tcycs != 0);

					// try 8, assume a1 = 0.8, assume a2 = 1 
					//nval = 0.8*p.U.pA.hreps;
					//oval = p.U.pA.mreps;

					// try 9, assume a1 = 0.6, assume a2 = 1 
					//nval = 0.6*p.U.pA.hreps;
					//oval = p.U.pA.mreps;

					// try 10, assume a1 = 0.8, assume a2 = 0.8
					//nval = 0.8*p.U.pA.hreps*(p.U.pA.mcycs + p.U.pA.tcycs);
					//oval = p.U.pA.treps*p.U.pA.tcycs + p.U.pA.mreps*p.U.pA.mcycs;

					// try 11, assume a1 = 0.6, assume a2 = 0.8
					// BEST
					nval = p.U.pA.hreps*(0.6*p.U.pA.mcycs + 0.8*p.U.pA.tcycs);
					oval = p.U.pA.treps*p.U.pA.tcycs + p.U.pA.mreps*p.U.pA.mcycs;

					// try 12, assume a1 = 0.8, assume a2 = 0.6
					// try 13, assume a1 = 0.6, assume a2 = 0.6
					//nval = 0.6*p.U.pA.hreps*(p.U.pA.mcycs + p.U.pA.tcycs);
					//oval = p.U.pA.treps*p.U.pA.tcycs + p.U.pA.mreps*p.U.pA.mcycs;

					if (nval <= oval) {
						#ifdef COMBINE_DEBUG
						errs() << nval << " <= " << oval << " DO NOT DISCARD A\n";
						#endif
						return true;
					}
					#ifdef COMBINE_DEBUG
					errs() << nval << " > " << oval << " DISCARD A\n";
					#endif

					// for profiling
					assert(pfp != NULL);
					fprintf(pfp, "Pattern A structure removed based on current heuristics\n");

					//return false;
					/*
					if ((p.U.pA.hreps)/(p.U.pA.mreps) < THRESH_A) {
						return true;
					}
					if (p.U.pA.hreps == p.U.pA.mreps) {
						return true;
					}*/
					break;
				case 'B':
					assert(p.U.pB.hcycs != 0 && p.U.pB.lcycs != 0 && p.U.pB.rcycs != 0 && p.U.pB.tcycs != 0);
					
					// sweeping for A, never combine B
					//return false;

					//errs() << "The max of ";
					//errs() << p.U.pB.lcycs << " and " << p.U.pB.rcycs;
					//errs() << " is " << max(p.U.pB.lcycs, p.U.pB.rcycs) << " \n";

					// try 1, assume a1 = 0.8, assume a2 = 1
					//nval = 0.8*p.U.pB.hreps*max(p.U.pB.lcycs, p.U.pB.rcycs);
					//oval = p.U.pB.lreps*p.U.pB.lcycs + p.U.pB.rreps*p.U.pB.rcycs;

					// try 2, assume a1 = 0.6, assume a2 = 1
					//nval = 0.6*p.U.pB.hreps*max(p.U.pB.lcycs, p.U.pB.rcycs);
					//oval = p.U.pB.lreps*p.U.pB.lcycs + p.U.pB.rreps*p.U.pB.rcycs;
					
					// try 3, assume a1 = 0.8, assume a2 = 0.8
					//nval = 0.8*p.U.pB.hreps*(max(p.U.pB.lcycs, p.U.pB.rcycs) + p.U.pB.tcycs);
					//oval = p.U.pB.lreps*p.U.pB.lcycs + p.U.pB.rreps*p.U.pB.rcycs + p.U.pB.treps*p.U.pB.tcycs;

					// try 4, assume a1 = 0.6, assume a2 = 0.8
					//nval = p.U.pB.hreps*(0.6*max(p.U.pB.lcycs, p.U.pB.rcycs) + 0.8*p.U.pB.tcycs);
					//oval = p.U.pB.lreps*p.U.pB.lcycs + p.U.pB.rreps*p.U.pB.rcycs + p.U.pB.treps*p.U.pB.tcycs;

					// try 5, assume a1 = 0.8, assume a2 = 0.6
					// BEST
					nval = p.U.pB.hreps*(0.8*std::max(p.U.pB.lcycs, p.U.pB.rcycs) + 0.6*p.U.pB.tcycs);
					oval = p.U.pB.lreps*p.U.pB.lcycs + p.U.pB.rreps*p.U.pB.rcycs + p.U.pB.treps*p.U.pB.tcycs;

					// try 6, assume a1 = 0.6, assume a2 = 0.6
					//nval = p.U.pB.hreps*(0.6*max(p.U.pB.lcycs, p.U.pB.rcycs) + 0.6*p.U.pB.tcycs);
					//oval = p.U.pB.lreps*p.U.pB.lcycs + p.U.pB.rreps*p.U.pB.rcycs + p.U.pB.treps*p.U.pB.tcycs;

					// try 7, assume a1 = 0.4, assume a2 = 0.4
					//nval = p.U.pB.hreps*(0.4*max(p.U.pB.lcycs, p.U.pB.rcycs) + 0.4*p.U.pB.tcycs);
					//oval = p.U.pB.lreps*p.U.pB.lcycs + p.U.pB.rreps*p.U.pB.rcycs + p.U.pB.treps*p.U.pB.tcycs;

					if (nval <= oval) {
						#ifdef COMBINE_DEBUG
						errs() << nval << " <= " << oval << " DO NOT DISCARD\n";
						#endif
						return true;
					}
					#ifdef COMBINE_DEBUG
					errs() << nval << " > " << oval << " DISCARD\n";
					#endif

					// for profiling
					assert(pfp != NULL);
					fprintf(pfp, "Pattern B structure removed based on current heuristics\n");

					// keep this patternB if both left and right cycles about the same
					//
					// max(lcyc, rcyc)/abs(lcyc-rcyc) > THRESH_B
					 
					/*
					if ((p.U.pB.lcycs) > (p.U.pB.rcycs)) { // l is greater than r
						if ((p.U.pB.lcycs)/(p.U.pB.lcycs - p.U.pB.rcycs) > THRESH_B) {
							return true;
						}
					} else if ((p.U.pB.rcycs) > (p.U.pB.lcycs)) { // r is greater than l
						if ((p.U.pB.rcycs)/(p.U.pB.rcycs - p.U.pB.lcycs) > THRESH_B) {
							return true;
						}
					} else { // l == r
						return true;
					}*/
					break;
				default:
					break;
			}
			fclose (pfp);
			return false;
		}


		/* find_combine_patterns()
		 * Find and label the patterns which should be merged in the function F
		 * Return the pointer to a linked list containing the basic blocks which should be combined
		 * Return NULL if no pattern found
		 */
		void LegUpCombineBB::find_combine_patterns(Function &F, std::vector<CombineList> &clist, int combine) {
			LoopInfo &LoopInfoObj = getAnalysis<LoopInfo>();
			// iterate through each basic block of function
			for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; ++BB) {
				if (combine == 1) { // initialize the hash only if first time merge
					// add basicblock label to hash, as is
					basicblock_hash.insert(std::pair<StrPair, StrPair>(std::make_pair(F.getName(), getLabel(BB)), std::make_pair(F.getName(), getLabel(BB))));
					num_total_blks += 1;
				}

				// for each basic block, determine whether it is the tail block of a pattern
				// If loops_only is true, return null ptr if any of the basic blocks is not within a loop
				BasicBlock *headA = is_patternA_tail(BB, LoopInfoObj);
				BasicBlock *headB = is_patternB_tail(BB, LoopInfoObj);
				if (headA != NULL) {
					// pattern A is eligible for reduction, check for store has been performed
					// add pattern A to list
					add_pattern_to_list(F, clist, headA, BB, 'A', combine);
				} else if (headB != NULL) {
					// pattern B is eligible for reduction, check for store has been performed
					// add pattern B to list
					add_pattern_to_list(F, clist, headB, BB, 'B', combine);
				}
			}
			return;
		}

		/* is_patternA_tail()
		 * returns a pointer to the head basic block if basic block B is the tail block of patternA
		 * else return NULL
		 */
		BasicBlock *LegUpCombineBB::is_patternA_tail(BasicBlock *B, LoopInfo &LoopInfoObj) {
			assert(B != NULL);
			int count = 0;
			BasicBlock *p1 = NULL, *p2 = NULL;
			pred_iterator PI, PE;

			for (PI = pred_begin(B), PE = pred_end(B); PI != PE; ++PI) {
				count++;
				p1 = p2;
				p2 = *PI;
			}

			// need exactly 2 different predecessors
			if (count != 2 || p1 == p2) return 0;

			if (loops_only && (LoopInfoObj.getLoopFor(B) == 0 || LoopInfoObj.getLoopFor(p1) == 0 || LoopInfoObj.getLoopFor(p2) == 0)) {
				return 0;
			}

			if (p2->getUniquePredecessor() == p1) {
				// p1 is the unique predecessor of p2
				
				if (is_patternA(p1, p2, B)) {
					return p1;
				}
			} else if (p1->getUniquePredecessor() == p2) {
				// p2 is the unique predecessor of p1
				
				if (is_patternA(p2, p1, B)) {
					return p2;
				}
			}
			return 0;
		}


		bool LegUpCombineBB::is_patternA(BasicBlock *BBH, BasicBlock *BBM, BasicBlock *BBT) {
			// BBH is the unique predecessor of BBM
			
			// for profiling
			FILE *pfp = fopen("combine_profile.rpt", "a");
			assert(pfp != NULL);
			
			// check that BBM only has 1 successor, BBT
			if (BBM->getTerminator()->getNumSuccessors() != 1) {
				if (!ProfileOutput) {
					fprintf(pfp, "Pattern A disqualified: Intermediate block has more than 1 successor\n");	
				}
				return false;
			}
			// check that BBH only has 2 successors, BBT and BBM
			if (BBH->getTerminator()->getNumSuccessors() != 2) {
				if (!ProfileOutput) {
					fprintf(pfp, "Pattern A disqualified: Head block has more than 2 successors\n");	
				}
				return false;
			}
			// cannot have this loop
			if (BBH == BBT) return false;
				if (!ProfileOutput) {
				fprintf(pfp, "Pattern A structure detected\n");
			}
			// final check for possible memory writes
			#ifdef PRED_STORE
			if (BasicBlockContainsCallInst(BBM)) {
				// for profiling
				if (!ProfileOutput) {
					fprintf(pfp, "Pattern A structure contains call instruction\n");
				}
				fclose(pfp);
				return false;
			}
			#else
			if (BasicBlockMayWriteToMemory(BBM)) {
				// for profiling
				if (!ProfileOutput) {
					fprintf(pfp, "Pattern A structure contains memory modification instruction\n");
				}
				fclose(pfp);
				return false;
			}
			#endif
			fclose(pfp);
			return true;
		}

		/* is_patternB_tail()
		 * returns a pointer to the head basicblock if basic block B is the tail block of patternB
		 * else return NULL
		 */
		BasicBlock *LegUpCombineBB::is_patternB_tail(BasicBlock *B, LoopInfo &LoopInfoObj) {
			assert(B != NULL);
			
			// for profiling
			FILE *pfp = fopen("combine_profile.rpt", "a");
			assert(pfp != NULL);

			int count = 0;
			BasicBlock *p1 = NULL, *p2 = NULL;
			pred_iterator PI, PE;

			for (PI = pred_begin(B), PE = pred_end(B); PI != PE; ++PI) {
				count++;
				p1 = p2;
				p2 = *PI;
			}

			// need exactly 2 different predecessors
			if (count != 2 || p1 == p2) {
				return 0;
			}

			BasicBlock *pp1, *pp2;
			pp1 = p1->getUniquePredecessor();
			pp2 = p2->getUniquePredecessor();

			// skip if either p1 or p2 have more than 1 pred, or unique predecessors are not equal, or one of them has no predecessors
			if (pp1 != pp2 || pp1 == NULL || pp2 == NULL) {
				// special case: entry nodes have no predecessors
				if (pred_begin(p1) == pred_end(p1) && pred_begin(p2) == pred_end(p2)) ;
				else return 0;
			}
			
			if (loops_only && (LoopInfoObj.getLoopFor(B) == 0 || LoopInfoObj.getLoopFor(p1) == 0 || LoopInfoObj.getLoopFor(p2) == 0 || LoopInfoObj.getLoopFor(pp1) == 0)) {
				return 0;
			}

			if (is_patternB(pp1, p1, p2, B) == 0) {
				return 0;
			}

			fclose(pfp);
			return pp1;
		}

		/* is_patternB()
		 * Returns true if BBH, BBL, BBR, and BBT make up a patternB formation:
		 *
		 *			BBH
		 *			/ \
		 *		  BBL BBR
		 *			\ /
		 *			BBT
		 *
		 * And no other edges exist between these 4 basicblocks in the CFG
		 * BBL and BBR are interchangeable
		 */
		bool LegUpCombineBB::is_patternB(BasicBlock *BBH, BasicBlock *BBL, BasicBlock *BBR, BasicBlock *BBT) {
			FILE *pfp = fopen("combine_profile.rpt", "a");
			assert(pfp != NULL);

			// check that BBL and BBR both have exactly 1 successor
			if (BBL->getTerminator()->getNumSuccessors() != 1 || BBR->getTerminator()->getNumSuccessors() != 1) {
				if (!ProfileOutput) {
					fprintf(pfp, "Pattern B disqualified: One or more Intermediate block has more than 1 successors\n");
				}
				return false;
			}

			// check that BBH has exactly 2 successors
			if (BBH->getTerminator()->getNumSuccessors() != 2) {
				if (!ProfileOutput) {
					fprintf(pfp, "Pattern B disqualified: Head block has more than 2 successors\n");
				}
				return false;
			}
			// cannot have this loop
			if (BBH == BBT) return false;
			if (!ProfileOutput) {
				fprintf(pfp, "Pattern B structure detected\n");
			}
			// final check for possible memory writes
			#ifdef PRED_STORE
			if (BasicBlockContainsCallInst(BBL) || BasicBlockContainsCallInst(BBR)) { //TODO yet to implement for pattern B
				// for profiling
				if (!ProfileOutput) {
					fprintf(pfp, "Pattern B structure contains call instruction\n");
				}
				fclose(pfp);
				return false;
			}
			#else
			if (BasicBlockMayWriteToMemory(BBL) || BasicBlockMayWriteToMemory(BBR)) {
				// for profiling
				if (!ProfileOutput) {
					fprintf(pfp, "Pattern B structure contains memory modification instruction\n");
				}
				fclose(pfp);
				return false;
			}
			#endif

			fclose(pfp);
			return true;
		}

		/* add_pattern_to_list()
		 * adds the new pattern (A/B) to the beginning of the linked list
		 * where B is a pointer to the tail block of the pattern
		 */
		void LegUpCombineBB::add_pattern_to_list(Function &F, std::vector<CombineList> &list, BasicBlock *H, BasicBlock *B, char type, int combine) {

			assert(B != NULL && H != NULL);
			CombineList node;
			node.type = type;
			node.overlap = false;

			// for each different type, find the basic blocks of the pattern
			switch (type) {
				case 'A':
					node.U.pA.head = H;
					if (B == H->getTerminator()->getSuccessor(1)) {
						node.U.pA.middle = H->getTerminator()->getSuccessor(0); // 'true' target
					} else {
						node.U.pA.middle = H->getTerminator()->getSuccessor(1); // 'false' target
					}
					node.U.pA.tail = B;
					node.U.pA.hcycs = cyc_hash[std::make_pair(F.getName(), getLabel(node.U.pA.head))];
					node.U.pA.mcycs = cyc_hash[std::make_pair(F.getName(), getLabel(node.U.pA.middle))];
					node.U.pA.tcycs = cyc_hash[std::make_pair(F.getName(), getLabel(node.U.pA.tail))];
					node.U.pA.hreps = rep_hash[std::make_pair(F.getName(), getLabel(node.U.pA.head))];
					node.U.pA.mreps = rep_hash[std::make_pair(F.getName(), getLabel(node.U.pA.middle))];
					node.U.pA.treps = rep_hash[std::make_pair(F.getName(), getLabel(node.U.pA.tail))];
					// some sanity checks
					assert(node.U.pA.hreps == node.U.pA.treps); // head and tail execute same number of times
					assert(node.U.pA.hreps >= node.U.pA.mreps); // middle executes at most the number of times head executes
					
					if (combine == 1) {
						update_basicblock_hash_patternA(F, list, node, num_merged_blks, num_merged_patterns);
					}

					break;
				case 'B':
					node.U.pB.head = H;
					node.U.pB.left = H->getTerminator()->getSuccessor(0); // 'true' target
					node.U.pB.right = H->getTerminator()->getSuccessor(1); // 'false' target
					node.U.pB.tail = B;
					node.U.pB.hcycs = cyc_hash[std::make_pair(F.getName(), getLabel(node.U.pB.head))];
					node.U.pB.lcycs = cyc_hash[std::make_pair(F.getName(), getLabel(node.U.pB.left))];
					node.U.pB.rcycs = cyc_hash[std::make_pair(F.getName(), getLabel(node.U.pB.right))];
					node.U.pB.tcycs = cyc_hash[std::make_pair(F.getName(), getLabel(node.U.pB.tail))];
					node.U.pB.hreps = rep_hash[std::make_pair(F.getName(), getLabel(node.U.pB.head))];
					node.U.pB.lreps = rep_hash[std::make_pair(F.getName(), getLabel(node.U.pB.left))];
					node.U.pB.rreps = rep_hash[std::make_pair(F.getName(), getLabel(node.U.pB.right))];
					node.U.pB.treps = rep_hash[std::make_pair(F.getName(), getLabel(node.U.pB.tail))];
					// some sanity checks
					assert(node.U.pB.hreps == node.U.pB.treps); // head and tail execute same number of times
					assert(node.U.pB.hreps >= node.U.pB.lreps); // left executes at most the number of times head executes
					assert(node.U.pB.hreps >= node.U.pB.rreps); // right executes at most the number of times head executes

					assert(node.type == 'B');
					if (combine == 1) {
						update_basicblock_hash_patternB(F, list, node, num_merged_blks, num_merged_patterns);
					}

					break;
				default:
					assert(0);
					break;
			}
			list.push_back(node);
			return;
		}


		/*
		 *
		 *
		 */
		void LegUpCombineBB::update_basicblock_hash_patternA(Function &F, std::vector<CombineList> &list, CombineList &node, int &num_merged_blks, int &num_merged_patterns) {
			assert(node.type == 'A');
			// update basicblock_hash
			std::string tail_label = getLabel(node.U.pA.tail).erase(0,1);
			if (is_number(tail_label)) { // tail blk label numeric SSA
				if (head_belongs_to_previous_pattern(F, list, node.U.pA.head)) {
					node.overlap = true;
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.middle))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.head))];
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.tail))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.head))];
					/* if head is already part of a previous pattern (as the tail),
					 * then the total number of merged basic blocks is one less
					 */
					num_merged_blks += 2;
					//num_merged_patterns += 0;
				} else {
					int tmp = (atoi((getLabel(node.U.pA.head).erase(0,1)).c_str()) - num_merged_blks + num_merged_patterns);
					char name[100];
					sprintf(name, "%d", tmp);
					std::string new_label(name);
					new_label = "%" + new_label;
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.head))] = std::make_pair(F.getName(), new_label);
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.middle))] = std::make_pair(F.getName(), new_label);
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.tail))] = std::make_pair(F.getName(), new_label);
					num_merged_blks += 3;
					num_merged_patterns += 1;
				}
			} else { // tail blk label not numeric
				assert(node.type == 'A');
				basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.head))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.tail))];
				basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.middle))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pA.tail))];
				num_merged_blks += 3;
				num_merged_patterns += 1;

				// need to back propagate non-numeric label to previously overlapped blocks
				if (head_belongs_to_previous_pattern(F, list, node.U.pA.head)) {
					#ifdef COMBINE_DEBUG
					errs() << "PROPAGATE THE HEAD UP " << getLabel(node.U.pA.head) << "\n";
					#endif
					propagate_label_to_previous_patterns(F, list, node.U.pA.head);
				}
			}

			assert(node.type == 'A');
		}


		void LegUpCombineBB::update_basicblock_hash_patternB(Function &F, std::vector<CombineList> &list, CombineList &node, int &num_merged_blks, int &num_merged_patterns) {
			assert(node.type == 'B');
			// update basicblock_hash
			std::string tail_label = getLabel(node.U.pB.tail).erase(0,1);
			if (is_number(tail_label)) { // tail blk label numeric SSA
				if (head_belongs_to_previous_pattern(F, list, node.U.pB.head)) {
					node.overlap = true;
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.left))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.head))];
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.right))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.head))];
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.tail))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.head))];
					/* if head is already part of a previous pattern (as the tail),
					 * then the total number of merged basic blocks is one less
					 */
					num_merged_blks += 3;
					//num_merged_patterns += 0;
				} else {
					int tmp = ((atoi((getLabel(node.U.pB.head).erase(0,1)).c_str()) - num_merged_blks + num_merged_patterns));
					#ifdef COMBINE_DEBUG
					errs() << "\n\t" << atoi((getLabel(node.U.pB.head).erase(0,1)).c_str());
					errs() << "\n\t" << num_merged_blks;
					errs() << "\n\t" << num_merged_patterns;
					errs() << "\n\t" << tmp;
					#endif
					char name[100];
					sprintf(name, "%d", tmp);
					std::string new_label(name);
					new_label = "%" + new_label;
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.head))] = std::make_pair(F.getName(), new_label);
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.left))] = std::make_pair(F.getName(), new_label);
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.right))] = std::make_pair(F.getName(), new_label);
					basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.tail))] = std::make_pair(F.getName(), new_label);
					num_merged_blks += 4;
					num_merged_patterns += 1;
				}
			} else { // tail blk label not numeric
				assert(node.type == 'B');
				basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.head))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.tail))];
				basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.left))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.tail))];
				basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.right))] = basicblock_hash[std::make_pair(F.getName(), getLabel(node.U.pB.tail))];
				num_merged_blks += 4;
				num_merged_patterns += 1;

				// need to back propagate non-numeric label to previously overlapped blocks
				if (head_belongs_to_previous_pattern(F, list, node.U.pB.head)) {
					#ifdef COMBINE_DEBUG
					errs() << "PROPAGATE THE HEAD UP " << getLabel(node.U.pB.head) << "\n";
					#endif
					propagate_label_to_previous_patterns(F, list, node.U.pB.head);
				}
			}
			assert(node.type == 'B');
		}

		/*
		 * Given the list of merge-able patterns detected in the program,
		 * and a pointer to the head of the current pattern being examined,
		 * return true if the head block exists as the tail block of some other node,
		 * otherwise return false.
		 */
		bool LegUpCombineBB::head_belongs_to_previous_pattern(Function &F, std::vector<CombineList> &list, BasicBlock *head)
		{
			if (list.size() == 0) {
				return false;
			}
			int i = 1;
			for (std::vector<CombineList>::iterator it = list.begin(); it != list.end(); i++, it++) {
				switch (it->type) {
					case 'A':
						if (head == it->U.pA.tail) {
							#ifdef COMBINE_DEBUG
							errs() << "A head is tail\n";
							#endif
							// update the basicblock_hash so that the head has same mapping as prev head
							return true;
						}
						// sanity checks
						assert(head != it->U.pA.middle);
						assert(head != it->U.pA.tail);
						break;
					case 'B':
						if (head == it->U.pB.tail) {
							#ifdef COMBINE_DEBUG
							errs() << "B head is tail\n";
							#endif
							// update the basicblock_hash so that the head has same mapping as prev head
							return true;
						}
						// sanity checks
						assert(head != it->U.pB.left);
						assert(head != it->U.pB.right);
						assert(head != it->U.pB.tail);
						break;
					default:
						errs() << "What type of pattern is this?!\n";
						assert(0);
						break;
				}
			}
			return false;
		}


		/* Given the function, list of patterns to merge, and head basic block of next pattern
		 * where the new pattern will map to a non-numeric label, propagate this name backward
		 * and update the basic block mappings for all basicblocks affected
		 */
		void LegUpCombineBB::propagate_label_to_previous_patterns(Function &F, std::vector<CombineList> &list, BasicBlock *head)
		{

			std::vector<CombineList>::reverse_iterator rit;
			BasicBlock *curr_head = head;

			bool propagate = true;
			while (propagate == true) {
				propagate = false;
				for (rit = list.rbegin(); rit != list.rend(); ++rit) { // probably would have the most luck from the end
					if (rit->type == 'A') {
						if (rit->U.pA.tail == curr_head) {
							propagate = true;
							basicblock_hash[std::make_pair(F.getName(), getLabel(rit->U.pA.middle))] = basicblock_hash[std::make_pair(F.getName(), getLabel(head))];
							basicblock_hash[std::make_pair(F.getName(), getLabel(rit->U.pA.head))] = basicblock_hash[std::make_pair(F.getName(), getLabel(head))];
							// don't need to do for tail, same as head
							curr_head = rit->U.pA.head;
							break; // the head can only overlap one tail
						}
					} else if (rit->type == 'B') {
						if (rit->U.pB.tail == curr_head) {
							propagate = true;
							basicblock_hash[std::make_pair(F.getName(), getLabel(rit->U.pB.right))] = basicblock_hash[std::make_pair(F.getName(), getLabel(head))];
							basicblock_hash[std::make_pair(F.getName(), getLabel(rit->U.pB.left))] = basicblock_hash[std::make_pair(F.getName(), getLabel(head))];
							basicblock_hash[std::make_pair(F.getName(), getLabel(rit->U.pB.head))] = basicblock_hash[std::make_pair(F.getName(), getLabel(head))];
							// don't need to do for tail, same as head
							curr_head = rit->U.pB.head;
							break; // the head can only overlap one tail
						}
					} else {
						assert(0);
					}
				}
			}
			return;
		}
		
		
		/* Given pointer to null terminated char array, return 1 if characters are all numeric,
		 * otherwise return 0
		 */
		bool LegUpCombineBB::is_number (std::string &str) {
			unsigned int i;
			for (i = 0; i < str.length(); i++) {
				if (! (str[i] >= '0' && str[i] <= '9')) {
					return false;
				}
			}
			return true;
		}

		void LegUpCombineBB::print_CombineList(std::vector<CombineList> &list) {
			FILE *pfp = fopen("combine_profile.rpt", "a");
			assert(pfp != NULL);

			if (list.size() == 0) {
				if (!ProfileOutput) {
					fprintf(pfp, "Nothing in CombineList\n");
				}
			} else {
				std::vector<CombineList>::iterator p;
				int i = 0;
				for (p = list.begin(); p != list.end(); p++) {
					i++;
					if (p->type == 'A' && !ProfileOutput) {
						fprintf(pfp, "CombineList A%d: %s - %s - %s\n", i, getLabel(p->U.pA.head).c_str(), getLabel(p->U.pA.middle).c_str(), getLabel(p->U.pA.tail).c_str());
					} else if (p->type == 'B' && !ProfileOutput) {
						fprintf(pfp, "CombineList B%d: %s - %s - %s - %s\n", i, getLabel(p->U.pB.head).c_str(), getLabel(p->U.pB.left).c_str(), getLabel(p->U.pB.right).c_str(), getLabel(p->U.pB.tail).c_str());
					}
				}
			}

			fclose(pfp);
		}

		/* debug_print_CombineList()
		 * print list for debugging
		 */
		void LegUpCombineBB::debug_print_CombineList(std::vector<CombineList> &list, int v) {
			if (list.size() == 0) {
				#ifdef COMBINE_DEBUG
				errs() << "Nothing in list\n";
				#endif
				return;
			}
			#ifdef COMBINE_DEBUG
			errs() << "Size of list = " << list.size() << "\n";
			int i = 0;
			#endif
			int a = 0, b = 0;
			for (std::vector<CombineList>::iterator p = list.begin(); p != list.end(); ++p) {
				#ifdef COMBINE_DEBUG
				errs() << "List Element " << ++i << "\n";
				errs() << "Type " << p->type << "\n";
				#endif
				if (p->type == 'A') {
					a++;
					#ifdef COMBINE_DEBUG
					errs() << "H : " << getLabel(p->U.pA.head) << " " << p->U.pA.hcycs << " " << p->U.pA.hreps  << /*" " << *(p->U.pA.head) << */ "\n";
					errs() << "M : " << getLabel(p->U.pA.middle) << " " << p->U.pA.mcycs << " " << p->U.pA.mreps  << /*" " << *(p->U.pA.middle) << */ "\n";
					errs() << "T : " << getLabel(p->U.pA.tail) << " " << p->U.pA.tcycs << " " << p->U.pA.treps  << /*" " << *(p->U.pA.tail) << */ "\n";
					#endif
				} else if (p->type == 'B') {
					b++;
					#ifdef COMBINE_DEBUG
					errs() << "H : " << getLabel(p->U.pB.head) << " " << p->U.pB.hcycs << " " << p->U.pB.hreps << /*" " << *(p->U.pB.head) << */ "\n";
					errs() << "L : " << getLabel(p->U.pB.left) << " " << p->U.pB.lcycs << " " << p->U.pB.lreps << /*" " << *(p->U.pB.left) << */ "\n";
					errs() << "R : " << getLabel(p->U.pB.right) << " " << p->U.pB.rcycs << " " << p->U.pB.rreps << /*" " << *(p->U.pB.right) << */ "\n";
					errs() << "T : " << getLabel(p->U.pB.tail) << " " << p->U.pB.tcycs << " " << p->U.pB.treps << /*" " << *(p->U.pB.tail) << */ "\n";
					#endif
				}
			}
			if (!v) {
				errs() << "======================================\n";
				errs() << "Initial Total # of patternA found : " << a << "\n";
				errs() << "Initial Total # of patternB found : " << b << "\n";
				errs() << "======================================\n";
			} else {
				errs() << "======================================\n";
				errs() << "Final Total # of patternA found : " << a << "\n";
				errs() << "Final Total # of patternB found : " << b << "\n";
				errs() << "======================================\n";
			}
			return;
		}


		/* merge_patterns_in_list()
		 * performs the actual merging of basic blocks from patterns
		 * defined in clist
		 */
		void LegUpCombineBB::merge_patterns_in_list(Function &F, std::vector<CombineList> &list)
		{
			if (list.size() == 0) {
				// no merging required, nothing on list
				return;
			}

			int a = 0, b = 0;
			for (std::vector<CombineList>::iterator p = list.begin(); p != list.end(); ++p) {
				switch (p->type) {
					case 'A':
						merge_basicblock_patternA(F, p->U.pA.head, p->U.pA.middle, p->U.pA.tail);
						a++;
						break;
					case 'B':
						merge_basicblock_patternB(F, p->U.pB.head, p->U.pB.left, p->U.pB.right, p->U.pB.tail);
						b++;
						break;
					default: break;
				}
			}

			#ifdef COMBINE_DEBUG
			errs() << "======================================\n";
			errs() << "Merged Pattern A: " << a << "\n";
			errs() << "Merged Pattern B: " << b << "\n";
			errs() << "======================================\n";
			#endif

			return;
		}

		        
		// BBH - head basic block
		// BBM - intermediate basic block
		// BBT - tail basic block
		bool LegUpCombineBB::merge_basicblock_patternA(Function &F, BasicBlock *BBH, BasicBlock *BBM, BasicBlock *BBT) {
			// bool profile = (atoi(getenv("PROFILE_BB")) == 1);
			assert(BBH != NULL && BBM != NULL && BBT != NULL);

			BasicBlock::iterator II, IE;
			BranchInst *BI;

			TerminatorInst *h_term = BBH->getTerminator();
			assert(h_term != NULL && isa<BranchInst>(*h_term));
			BI = dyn_cast<BranchInst>(h_term);
			assert(BI->isConditional());
			Value *cond = BI->getCondition(); // save br insn condition
			Value *TrueVal = BI->getSuccessor(0);
			Value *FalseVal = BI->getSuccessor(1);
			int condflag;
			if (TrueVal == BBT) { // true branches to BBT, false branches to BBM
			    assert(FalseVal == BBM);
				condflag = 0;
			} else if (TrueVal == BBM) { // true branches to BBM, false branches BBT
			    assert(FalseVal == BBT);
				condflag = 1;
			} else {
			    assert(0);
			}
			h_term->eraseFromParent();

			#ifdef PRED_STORE
			merge_basicblock_with_store(F, BBM, cond, condflag);
			#endif

			// remove last uncond branch from BBM
			TerminatorInst *m_term = BBM->getTerminator();
			assert(m_term != NULL && isa<BranchInst>(*m_term));
			BI = dyn_cast<BranchInst>(m_term);
			assert(BI->isUnconditional());
			m_term->eraseFromParent();

            Instruction *selectInst;
			Instruction *phiInst;
			PHINode *PN;
			Value *S1;
			Value *S2; // Assumption that phi only receives 2 values (should be correct...TODO)
			// Replace phi insn in BBT with select insn
			for (II = BBT->begin(), IE = BBT->end(); II != IE; ++II) {
			    if (isa<PHINode>(II)) {
				    // extract PHI variables
					phiInst = dyn_cast<PHINode>(II);
					assert(phiInst != NULL);
					PN = dyn_cast<PHINode>(phiInst);
					if (condflag == 0) { 
					    S1 = PN->getIncomingValueForBlock(BBH);
					    S2 = PN->getIncomingValueForBlock(BBM);
					} else {
					    S1 = PN->getIncomingValueForBlock(BBM);
					    S2 = PN->getIncomingValueForBlock(BBH);
					}
				    selectInst = SelectInst::Create(cond, S1, S2);
					ReplaceInstWithInst(phiInst->getParent()->getInstList(), II, selectInst);
				}
			}

			// Move all insn to BBT
			BBT->getInstList().splice(BBT->begin(), BBM->getInstList());
			BBT->getInstList().splice(BBT->begin(), BBH->getInstList());

            BBH->replaceAllUsesWith(BBT);

		    BBH->removeFromParent();
			BBM->removeFromParent();

			return true;
		}


		// BBH - head basic block
		// BBL - arbitrarily left basic block
		// BBR - arbitrarily right basic block
		// BBT - tail basic block
		bool LegUpCombineBB::merge_basicblock_patternB(Function &F, BasicBlock *BBH, BasicBlock *BBL, BasicBlock *BBR, BasicBlock *BBT) {
			// bool profile = (atoi(getenv("PROFILE_BB")) == 1);
			assert(BBH != NULL && BBL != NULL && BBR != NULL && BBT != NULL);

			BasicBlock::iterator II, IE;

			TerminatorInst *h_term = BBH->getTerminator();
			assert(isa<BranchInst>(*h_term));
			BranchInst *BI = dyn_cast<BranchInst>(h_term);
			assert(BI->isConditional());
			// read terminating 'br' insn in BBH to get the condition
			Value *cond = BI->getCondition();
			Value *TrueVal = BI->getSuccessor(0);
			Value *FalseVal = BI->getSuccessor(1);
			int condflagL, condflagR;
			if (TrueVal == BBL) { // true branches to BBL, false branches to BBR
				assert(FalseVal == BBR);
				condflagL = 1;
				condflagR = 0;
			} else if (TrueVal == BBR) {
				assert(FalseVal == BBL);
				condflagL = 0;
				condflagR = 1;
			} else {
				assert(0);
			}
			h_term->eraseFromParent();

			#ifdef PRED_STORE
			merge_basicblock_with_store(F, BBL, cond, condflagL);
			merge_basicblock_with_store(F, BBR, cond, condflagR);
			#endif
			
			remove_last_branch_instruction(BBL);
			remove_last_branch_instruction(BBR);

			move_instructions_and_insert_PHI(BBL, BBR, BBT, cond, condflagR);

            // Move instructions from BBR, BBL and BBH to BBT
			BBT->getInstList().splice(BBT->begin(), BBR->getInstList());
			BBT->getInstList().splice(BBT->begin(), BBL->getInstList());
			BBT->getInstList().splice(BBT->begin(), BBH->getInstList());

			// Update predecessors of BBH to now point to BBT
			BBH->replaceAllUsesWith(BBT);

			// merged block looks like:
			//  ________
			// |  BBH	|
			// |________|
			// |  BBL	|
			// |________|
			// |  BBR	|
			// |________|
			// |  BBT	|
			// |________|

			BBL->removeFromParent();
			BBR->removeFromParent();
			BBH->removeFromParent();

			return true;
		}


		void LegUpCombineBB::move_instructions_and_insert_PHI(BasicBlock *BBL, BasicBlock *BBR, BasicBlock *BBT, Value *cond, int condflag) {
			BasicBlock::iterator II, IE;
			Value *S1;
			Value *S2;
			// move all insn in BBT to BBH
			// move all insn in BBH to BBT
		    Instruction* phiInst = NULL;
			for (II = BBT->begin(), IE = BBT->end(); II != IE; II++) {
				if (isa<PHINode>(II)) { // change phi insn in BBT to select, then move to BBH
					phiInst = dyn_cast<Instruction>(II);
					assert(phiInst != NULL);
					PHINode *PN = dyn_cast<PHINode>(phiInst);
					if (condflag == 0) {
						S1 = PN->getIncomingValueForBlock(BBL); // true val
						S2 = PN->getIncomingValueForBlock(BBR);
					} else {
						S1 = PN->getIncomingValueForBlock(BBR); // true val
						S2 = PN->getIncomingValueForBlock(BBL);
					}
					Instruction *selectInst = SelectInst::Create(cond, S1, S2);
					ReplaceInstWithInst(phiInst->getParent()->getInstList(), II, selectInst); // Replace phi insn with select and delete phi insn
				}
			}
			return;
		}

		/* merge_basicblock_with_store()
		 * Type specifies if the basicblock is BBM of patternA (MMERGE), BBL of patternB (LMERGE), or BBR of patternB (RMERGE)
		 */
		void LegUpCombineBB::merge_basicblock_with_store (Function &F, BasicBlock *BB, Value *cond, int condflag) {
			BasicBlock::iterator I, E;
			SelectInst *selectAddrInst;
			StoreInst *storeInst;
			AllocaInst *allocaInst = NULL;
			Value *nullVal = NULL;
			/*
			if (!ProfileOutput) {
				nullVal = ConstantPointerNull::get(PointerType::get(Type::getInt32Ty(F.getContext()), 0));
			}
			*/

			Value *A1;

			// take care of store instructions
			for (I = BB->begin(), E = BB->end(); I != E; ++I) {
				if (isa<StoreInst>(I)) {
					storeInst = dyn_cast<StoreInst>(I);
					A1 = storeInst->getPointerOperand();

					if (ProfileOutput) {
						if (allocaInst == NULL || allocaInst->getAllocatedType() != storeInst->getValueOperand()->getType()) {
							// only add new alloca instruction if different type encountered
							std::string allocaName = "alloca_" + F.getName().str();// + "_" + storeInst->getValueOperand()->getName().str();
							allocaInst = new AllocaInst(storeInst->getValueOperand()->getType());
							allocaInst->setName(Twine(allocaName));

							BB->getInstList().insert(BB->getFirstNonPHI(), allocaInst);
						}
					} else {					
						nullVal = ConstantPointerNull::get(PointerType::get(Type::getInt32Ty(F.getContext()), 0));
						if (A1->getType() != nullVal->getType()) {
							nullVal->mutateType(A1->getType());
						}
					}

					if (condflag == 0) {
						// Branch True <=> BBL
						// Branch False <=> BBR
						if (ProfileOutput) {
							selectAddrInst = SelectInst::Create(cond, allocaInst, A1);
						} else {
							selectAddrInst = SelectInst::Create(cond, nullVal, A1);
						}
					} else {
						// Branch True <=> BBR
						// Branch False <=> BBL
						if (ProfileOutput) {
							selectAddrInst = SelectInst::Create(cond, A1, allocaInst);
						} else {
							selectAddrInst = SelectInst::Create(cond, A1, nullVal);
						}
					}

					// insert select instruction before store
					BB->getInstList().insert(storeInst, selectAddrInst);
					storeInst->setOperand(1, selectAddrInst);
				}
			}
			return;
		}

		void LegUpCombineBB::remove_last_branch_instruction (BasicBlock *BB) {
			BasicBlock::iterator I, E;

			assert(isa<BranchInst>(*(BB->getTerminator())));
			assert((dyn_cast<BranchInst>(BB->getTerminator()))->isUnconditional());
			for (I = BB->begin(), E = BB->end(); I != E; I++) {
				if (isa<BranchInst>(*I)) {
					I->eraseFromParent();
					break;
				}
			}
			return;
		}


		// Returns true if BasicBlock contains an instruction which may write to memory
		// Returns false otherwise
		bool LegUpCombineBB::BasicBlockMayWriteToMemory(BasicBlock *BB) {
			for (BasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
			    Instruction *Inst = I;
				// try a more aggressive approach of only ignoring stores
				if (isa<StoreInst>(Inst)) {
					return true;
				}
				if (isa<CallInst>(Inst)) {
					// ignore any calls because they are black boxes!
					return true;
				}
			}
			return false;
		}// BasicBlockMayWriteToMemory

		
		// Returns true if BasicBlock contains a call instruction
		// Returns false otherwise
		bool LegUpCombineBB::BasicBlockContainsCallInst(BasicBlock *BB) {
			bool res = false;
			FILE *pfp = fopen("combine_profile.rpt", "a");
			assert(pfp != NULL);
			for (BasicBlock::iterator I = BB->begin(); I != BB->end(); ++I) {
			    Instruction *Inst = I;
				if (isa<StoreInst>(Inst)) {
					if (!ProfileOutput) {
						fprintf(pfp, "Basic block %s: store instruction\n", getLabel(BB).c_str());
					}
				}
				if (isa<CallInst>(Inst)) {
					// ignore any calls because they are black boxes!
					if (!ProfileOutput) {
						fprintf(pfp, "Basic block %s: call instruction\n", getLabel(BB).c_str());
					}
					res = true;
				}
			}
			fclose(pfp);
			return res;
		}// BasicBlockContainsCallInst

}// namespace legup

using namespace legup;

char LegUpCombineBB::ID = 0;
static RegisterPass<LegUpCombineBB> X("legup-combine-bb",
"Print list of BB's that can be combined");

