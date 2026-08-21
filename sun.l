%option noyywrap
%option header-file="sun.l.h"

%%
[[:graph:] ]+	{ return 0; }

[\t\n]	{ /* ignore non-space whitespace */ }
<<EOF>>	{ return -1; }
%%
