#ifndef _AFFIXMGR_HXX_
#define _AFFIXMGR_HXX_

#include "atypes.hxx"
#include "baseaffix.hxx"
#include "hashmgr.hxx"

// remove end of line char(s)
void mychomp(char * s);

// duplicate string
char * mystrdup(const char * s);

// parse string into tokens with char delimiter
char * mystrsep(char ** stringp, const char delim);

class AffixMgr
{
protected:
  int                 numpfx;
  int                 numsfx;
  Affix*              pTable[MAXAFFIXES];
  Affix*              sTable[MAXAFFIXES];
  char *              trystring;
  char *              encoding;

public:
 
  AffixMgr(const char * affpath);
  ~AffixMgr();
  struct hentry * affix_check(HashMgr *pHMgr, const char * word, int len) const;
  struct hentry * lookup(HashMgr *pHMgr, const char * word) const;
  struct hentry * cross_check(HashMgr *pHMgr, const char * word, int len, int sfxopts, const Affix* ppfx) const;
  char * get_encoding() const;
  char * get_try_string() const;
            
private:
  int  parse_file(const char * affpath);
  void encodeit(struct affentry * ptr, char * cs) const;
  void strtolower(char * cs) const;

};

#endif