#include "regex.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define FUTURA_REGEX_MAGIC 0x46524547UL


typedef struct { const char *p; const char *s; int flags; } RxCtx;

static int rx_eq(char a, char b, int flags) {
    if (flags & REG_ICASE) return tolower((unsigned char)a) == tolower((unsigned char)b);
    return a == b;
}

static const char *rx_find_close(const char *p) {
    int d=1, esc=0, cls=0;
    for (; *p; p++) {
        char c=*p;
        if (esc) { esc=0; continue; }
        if (c=='\\') { esc=1; continue; }
        if (c=='[') { cls=1; continue; }
        if (c==']' && cls) { cls=0; continue; }
        if (cls) continue;
        if (c=='(') d++;
        else if (c==')' && --d==0) return p;
    }
    return NULL;
}

static const char *rx_find_alt(const char *p, const char *end) {
    int d=0, esc=0, cls=0;
    for (; p<end && *p; p++) {
        char c=*p;
        if (esc) { esc=0; continue; }
        if (c=='\\') { esc=1; continue; }
        if (c=='[') { cls=1; continue; }
        if (c==']' && cls) { cls=0; continue; }
        if (cls) continue;
        if (c=='(') d++;
        else if (c==')' && d) d--;
        else if (c=='|' && d==0) return p;
    }
    return NULL;
}

static int rx_class(const char **pp, char ch, int flags) {
    const char *p=*pp; int neg=0, ok=0;
    if (*p!='[') return -1;
    p++;
    if (*p=='^' || *p=='!') { neg=1; p++; }
    while (*p && *p!=']') {
        if (*p=='[' && p[1]==':' ) {
            const char *q=strstr(p+2, ":]");
            if(q) {
                size_t n=(size_t)(q-(p+2));
                int yes=0; unsigned char uc=(unsigned char)ch;
                if(n==5 && strncmp(p+2,"digit",5)==0) yes=isdigit(uc)!=0;
                else if(n==5 && strncmp(p+2,"alpha",5)==0) yes=isalpha(uc)!=0;
                else if(n==5 && strncmp(p+2,"alnum",5)==0) yes=isalnum(uc)!=0;
                else if(n==5 && strncmp(p+2,"space",5)==0) yes=isspace(uc)!=0;
                else if(n==5 && strncmp(p+2,"lower",5)==0) yes=islower(uc)!=0;
                else if(n==5 && strncmp(p+2,"upper",5)==0) yes=isupper(uc)!=0;
                ok |= yes; p=q+2; continue;
            }
        }
        char a;
        if (*p=='\\' && p[1]) { a=p[1]; p+=2; }
        else a=*p++;
        if (*p=='-' && p[1] && p[1]!=']') {
            p++; char b=*p++;
            char aa=a,bb=b,cc=ch;
            if(flags&REG_ICASE){aa=tolower((unsigned char)aa);bb=tolower((unsigned char)bb);cc=tolower((unsigned char)cc);}
            if(cc>=aa && cc<=bb) ok=1;
        } else if(rx_eq(a,ch,flags)) ok=1;
    }
    if(*p!=']') return -1;
    *pp=p+1;
    return neg ? !ok : ok;
}

static int rx_atom_match(const char *p, const char *s, int flags, const char **after) {
    if(!*p || !*s) return 0;
    if(*p=='(') {
        const char *close=rx_find_close(p+1); if(!close) return -1;
        char buf[4096]; size_t n=(size_t)(close-(p+1)); if(n>=sizeof(buf)) return -1;
        memcpy(buf,p+1,n); buf[n]=0;
        regex_t r; if(regcomp(&r,buf,flags)!=0) return -1;
        /* Try every prefix, because a group is an atom with variable length. */
        for(const char *q=s;;q++) {
            regmatch_t m;
            if(regexec(&r,s,1,&m,0)==0 && m.rm_so==0 && s+m.rm_eo==q) { *after=q; regfree(&r); return 1; }
            if(*q==0) break;
        }
        regfree(&r); return 0;
    }
    if(*p=='[') {
        const char *q=p; int v=rx_class(&q,*s,flags); if(v<0)return -1; if(v){*after=s+1;return 1;} return 0;
    }
    if(*p=='.') { if((flags&REG_NEWLINE)&&*s=='\n')return 0; *after=s+1; return 1; }
    if(*p=='\\' && p[1]) { if(rx_eq(p[1],*s,flags)){*after=s+1;return 1;} return 0; }
    if(*p=='^' || *p=='$') return 0;
    if(rx_eq(*p,*s,flags)){*after=s+1;return 1;}
    return 0;
}

static int rx_match_range(const char *p, const char *pend, const char *s, int flags, const char **out);

static int rx_match_seq(const char *p, const char *pend, const char *s, int flags, const char **out) {
    if(p>=pend || *p==0) { *out=s; return 1; }
    if(*p=='$' && p+1==pend) { *out=s; return *s==0; }
    if(*p=='^') return rx_match_seq(p+1,pend,s,flags,out);
    const char *alt=rx_find_alt(p,pend);
    if(alt) {
        if(rx_match_seq(p,alt,s,flags,out)) return 1;
        return rx_match_seq(alt+1,pend,s,flags,out);
    }
    const char *atom_end=p+1;
    if(*p=='('){const char *c=rx_find_close(p+1);if(!c||c>=pend)return 0;atom_end=c+1;}
    else if(*p=='['){atom_end=p+1;while(atom_end<pend&&*atom_end!=']'){if(*atom_end=='\\'&&atom_end+1<pend)atom_end++;atom_end++;}if(atom_end<pend)atom_end++;}
    else if(*p=='\\'&&p+1<pend)atom_end=p+2;
    int min=1,max=1;
    if(atom_end<pend){
        if(*atom_end=='*'){min=0;max=1000000;atom_end++;}
        else if(*atom_end=='+'){min=1;max=1000000;atom_end++;}
        else if(*atom_end=='?'){min=0;max=1;atom_end++;}
        else if(*atom_end=='{'){
            const char*q=atom_end+1;int a=0,b=0;while(q<pend&&isdigit((unsigned char)*q)){a=a*10+(*q-'0');q++;}
            if(q<pend&&*q==','){q++;if(q<pend&&isdigit((unsigned char)*q)){while(q<pend&&isdigit((unsigned char)*q)){b=b*10+(*q-'0');q++;}}else b=1000000;}else b=a;
            if(q<pend&&*q=='}'){min=a;max=b;atom_end=q+1;}
        }
    }
    const char *pos[256]; int n=0; const char *cur=s;
    while(n<255 && n<max){
        const char *next; int r=rx_atom_match(p,cur,flags,&next);
        if(r!=1)break; pos[n++]=cur; cur=next;
    }
    if(n<min)return 0;
    for(int k=n;k>=min;k--){
        const char *try_s=(k==0)?s:cur;
        if(k<n) { const char *dummy; try_s=pos[k]; if(k>0){rx_atom_match(p,pos[k-1],flags,&dummy);try_s=dummy;} }
        if(rx_match_seq(atom_end,pend,try_s,flags,out))return 1;
    }
    return 0;
}

int regcomp(regex_t *r, const char *pattern, int flags) {
    if(!r||!pattern)return REG_BADPAT;
    size_t n=strlen(pattern);if(n>=sizeof(r->pattern))return REG_ESPACE;
    r->magic=FUTURA_REGEX_MAGIC;r->re_nsub=0;r->flags=flags;memcpy(r->pattern,pattern,n+1);return 0;
}
int regexec(const regex_t *r,const char *s,size_t nmatch,regmatch_t pm[],int flags){
    if(!r||r->magic!=FUTURA_REGEX_MAGIC||!s)return REG_BADPAT;
    const char *p=r->pattern;int anchored=(*p=='^');
    if(anchored)p++;
    size_t len=strlen(p);
    for(const char *start=s;;start++){
        const char *out;
        if(rx_match_seq(p,p+len,start,r->flags,&out)){
            if(nmatch&&pm){pm[0].rm_so=start-s;pm[0].rm_eo=out-s;}
            return 0;
        }
        if(anchored||*start==0)break;
    }
    return REG_NOMATCH;
}
void regfree(regex_t*r){if(r){r->magic=0;r->pattern[0]=0;}}
size_t regerror(int e,const regex_t*r,char*b,size_t n){const char*s=e==REG_NOMATCH?"no match":e==REG_ESPACE?"out of memory":"invalid regular expression";size_t l=strlen(s);if(b&&n){size_t c=l<n-1?l:n-1;memcpy(b,s,c);b[c]=0;}return l+1;}
