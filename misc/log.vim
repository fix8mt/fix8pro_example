"
"     ____                      __      ____
"    /\  _`\   __             /'_ `\   /\  _`\
"    \ \ \L\_\/\_\    __  _  /\ \L\ \  \ \ \L\ \ _ __    ___
"     \ \  _\/\/\ \  /\ \/'\ \/_> _ <_  \ \ ,__//\`'__\ / __`\
"      \ \ \/  \ \ \ \/>  </   /\ \L\ \  \ \ \/ \ \ \/ /\ \L\ \
"       \ \_\   \ \_\ /\_/\_\  \ \____/   \ \_\  \ \_\ \ \____/
"        \/_/    \/_/ \//\/_/   \/___/     \/_/   \/_/  \/___/
"
"                 Fix8Pro Example Client Server
"
" Copyright (C) 2010-22 Fix8 Market Technologies Pty Ltd (ABN 29 167 027 198)
" ALL RIGHTS RESERVED  https://www.fix8mt.com  heretohelp@fix8mt.com  @fix8mt
"
" This  file is released  under the  GNU LESSER  GENERAL PUBLIC  LICENSE  Version 3.  You can
" redistribute  it  and / or modify  it under the  terms of  the  GNU Lesser  General  Public
" License as  published  by  the Free  Software Foundation,  either version 3 of the License,
" or (at your option) any later version.
"
" This file is distributed in the hope that it will be useful, but  WITHOUT ANY WARRANTY  and
" without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
"
" You should  have received a copy of  the GNU Lesser General Public  License along with this
" file. If not, see <http://www.gnu.org/licenses/>.
"
" BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
" THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
" COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM 'AS IS' WITHOUT WARRANTY OF ANY
" KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
" WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
" THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
" YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
"
" IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
" HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
" ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
" CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
" NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
" THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
" HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
"
" fix8pro log file VIM syntax highlighting.
"
" Add the two lines below to you .vimrc - edit in the path to this file:
"     au! Syntax log so ~/log.vim
"     autocmd BufNewFile,BufRead *.log set syntax=log

let s:cpo_save = &cpoptions
set cpoptions&vim

hi thread_A guifg=#6495ed ctermfg=1
hi thread_B guifg=#ffd700 ctermfg=2
hi thread_C guifg=#d2b48c ctermfg=3
hi thread_D guifg=#8470ff ctermfg=6
hi thread_E guifg=#db7093 ctermfg=5
hi thread_F guifg=#228b22 ctermfg=4 cterm=bold
hi thread_G guifg=#f4a460 ctermfg=1 cterm=bold
hi thread_H guifg=#ff1493 ctermfg=2 cterm=bold
hi thread_I guifg=#b22222 ctermfg=3 cterm=bold
hi thread_J guifg=#48d1cc ctermfg=6 cterm=bold
hi thread_K guifg=#708090 ctermfg=5 cterm=bold
hi thread_L guifg=#7cfc00 ctermfg=4 cterm=underline
hi thread_M guifg=#468db4 ctermfg=1 cterm=underline
hi thread_N guifg=#5f9ea0 ctermfg=2 cterm=underline
hi thread_O guifg=#ffff00 ctermfg=3 cterm=underline
hi thread_W guifg=#ff69b4 ctermfg=4
hi thread_Q guifg=#66cdaa ctermfg=5 cterm=underline
hi thread_R guifg=#ffdab9 ctermfg=6 cterm=underline
hi thread_S guifg=#0000ff ctermfg=0 ctermbg=7
hi thread_T guifg=#2e8b57 ctermfg=1 ctermbg=7
hi thread_U guifg=#bdb76b ctermfg=2 ctermbg=7
hi thread_V guifg=#8b4513 ctermfg=3 ctermbg=7
hi thread_P guifg=#ffe4e1 ctermfg=4 ctermbg=7
hi thread_X guifg=#fffacd ctermfg=5 ctermbg=7
hi thread_Y guifg=#000000 ctermfg=6 ctermbg=7
hi thread_Z guifg=#00ffff ctermfg=7 ctermbg=0

hi def link logLevelCritical thread_T
hi def link logLevelError ErrorMsg
hi def link logLevelWarning WarningMsg
hi def link logLevelNotice Character
hi def link logLevelInfo Repeat
hi def link logLevelDebug thread_F
hi def link logLevelTrace Comment

syn keyword logLevelCritical CRITICAL CRIT FATAL Fatal
syn keyword logLevelError ERROR ERR FAILURE SEVERE Error
syn keyword logLevelWarning WARNING WARN Warn
syn keyword logLevelNotice NOTICE Notice
syn keyword logLevelInfo INFO Info
syn keyword logLevelDebug DEBUG FINE Debug
syn keyword logLevelTrace TRACE FINER FINEST
syn cluster ALLKEYHI contains=logLevelCritical,logLevelError,logLevelWarning,logLevelNotice,logLevelInfo,logLevelDebug,logLevelTrace

syn match thread_A /[0-9]\{7\}\sA\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_B /[0-9]\{7\}\sB\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_C /[0-9]\{7\}\sC\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_D /[0-9]\{7\}\sD\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_E /[0-9]\{7\}\sE\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_F /[0-9]\{7\}\sF\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_G /[0-9]\{7\}\sG\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_H /[0-9]\{7\}\sH\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_I /[0-9]\{7\}\sI\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_J /[0-9]\{7\}\sJ\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_K /[0-9]\{7\}\sK\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_L /[0-9]\{7\}\sL\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_M /[0-9]\{7\}\sM\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_N /[0-9]\{7\}\sN\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_O /[0-9]\{7\}\sO\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_P /[0-9]\{7\}\sP\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_Q /[0-9]\{7\}\sQ\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_R /[0-9]\{7\}\sR\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_S /[0-9]\{7\}\sS\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_T /[0-9]\{7\}\sT\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_U /[0-9]\{7\}\sY\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_V /[0-9]\{7\}\sV\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_W /[0-9]\{7\}\sW\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_X /[0-9]\{7\}\sX\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_Y /[0-9]\{7\}\sY\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI
syn match thread_Z /[0-9]\{7\}\sZ\s[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}\s[0-9]\{2\}:[0-9]\{2\}:[0-9]\{2\}.[0-9]\{9\}/me=e+999,hs=e+8,he=e+999 contains=@ALLKEYHI

