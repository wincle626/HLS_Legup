"
" VIM configuration for LegUp
" Mostly copied from llvm/utils/vim/vimrc, but with 4-space indentations
"
" You can use this file by sourcing it in your .vimrc file. eg.:
" source ~/legup/legup.vim
"

" It's VIM, not VI
set nocompatible

" A tab produces a 4-space indentation
set softtabstop=4
set shiftwidth=4
set expandtab

" Highlight trailing whitespace and lines longer than 80 columns.
highlight LongLine ctermbg=DarkYellow guibg=DarkYellow
highlight WhitespaceEOL ctermbg=DarkYellow guibg=DarkYellow
if v:version >= 702
    " Lines longer than 80 columns.
    if exists('+colorcolumn')
        set colorcolumn=81
    else
        au BufWinEnter * let w:m0=matchadd('LongLine', '\%>80v.\+', -1)
    endif

    " Whitespace at the end of a line. This little dance suppresses
    " whitespace that has just been typed.
    au BufWinEnter * let w:m1=matchadd('WhitespaceEOL', '\s\+$', -1)
    au InsertEnter * call matchdelete(w:m1)
    au InsertEnter * let w:m2=matchadd('WhitespaceEOL', '\s\+\%#\@<!$', -1)
    au InsertLeave * call matchdelete(w:m2)
    au InsertLeave * let w:m1=matchadd('WhitespaceEOL', '\s\+$', -1)
else
    " Lines longer than 80 columns.
    if exists('+colorcolumn')
        set colorcolumn=81
    else
        au BufRead,BufNewFile * syntax match LongLine /\%>80v.\+/
    endif

    " Whitespace at end of line
    au InsertEnter * syntax match WhitespaceEOL /\s\+\%#\@<!$/
    au InsertLeave * syntax match WhitespaceEOL /\s\+$/
endif

" Enable filetype detection
filetype on

" Optional
" C/C++ programming helpers
augroup csrc
    au!
    autocmd FileType *      set nocindent smartindent
    autocmd FileType c,cpp  set cindent
augroup END
" Set a few indentation parameters. See the VIM help for cinoptions-values for
" details.  These aren't absolute rules; they're just an approximation of
" common style in LLVM source.
set cinoptions=:0,g0,(0,Ws,l1
" Add and delete spaces in increments of `shiftwidth' for tabs
set smarttab

" Highlight syntax in programming languages
syntax on

" LLVM Makefiles can have names such as Makefile.rules or TEST.nightly.Makefile,
" so it's important to categorize them as such.
augroup filetype
    au! BufRead,BufNewFile *Makefile* set filetype=make
augroup END

" In Makefiles, don't expand tabs to spaces, since we need the actual tabs
autocmd FileType make set noexpandtab


" map Ctrl-K to run clang-format
" See: http://clang.llvm.org/docs/ClangFormat.html#vim-integration
map <C-K> :pyf clang-format.py<cr>
imap <C-K> <c-o>:pyf clang-format.py<cr>

