%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
  
int pos;
int len;
%}

binary 0[Bb][01]+

%%

{binary} { 
putchar('(');
len=strlen(yytext+2);

for(pos=len-1; pos>=1; pos--)
   if(*(yytext+2+len-pos-1)=='1')
      printf("(1<<%d)|", pos);

putchar(*(yytext+2+len-1));
putchar(')');
}

%%

main()
{ yylex(); return 0; }

