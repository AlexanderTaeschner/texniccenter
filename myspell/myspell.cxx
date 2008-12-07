#include "license.readme"

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "stdafx.h"
#include "myspell.hxx"
#include "character.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

MySpell::MySpell(const char * affpath, const char * dpath)
{
	/* first set up the main dict hash manager */
	pHMgr = new HashMgr(dpath);

	try {
		/* next set up the affix manager */
		pAMgr = new AffixMgr(affpath);
	} catch ( ... ) {
		// clean up and continue the exception
		delete pHMgr;
		throw;
	}

	/* get the preferred try string from the Affix Manager for that dictionary */
	char * try_string = pAMgr->get_try_string();

	/* and finally set up the suggestion manager */
	pSMgr = new SuggestMgr(try_string, 10, pAMgr);
	delete [] try_string;

	/* create the ignore and add hashes */
	pIgnoreHash = new HashMgr;
	pAddHash = new HashMgr;

	bModified = 0;
	bMainOnly = 1;
	strPersonalDictionaryPath = NULL;
}


MySpell::~MySpell()
{
	delete pSMgr;
	delete pAMgr;
	delete pHMgr;
	delete pIgnoreHash;
	delete pAddHash;
	delete [] strPersonalDictionaryPath;
}


int MySpell::captype(const char * word) const
{
	int ncap = 0;
	int nc = 0;
	const char * p = word;
	while (*p != '\0') {
		nc++;
		if (IsUpper(*p)) ncap++;
		p++;
	}
	if (ncap == 0) return NOCAP;
	if ((ncap == 1) && IsUpper(*word)) return INITCAP;
	if (ncap == nc) return ALLCAP;
	return HUHCAP;
}


void MySpell::mkallcap(char * p) const
{
	while (*p != '\0') { 
		*p = toupper((unsigned char) *p);
		p++;
	}
}


void MySpell::mkallsmall(char * p) const
{
	while (*p != '\0') {
		*p = tolower((unsigned char) *p);
		p++;
	}
}


int MySpell::spell(const char * word) const
{
	char * rv=NULL;
	char wspace[MAXWORDLEN+1];
	int wl = strlen(word);
	if (wl > MAXWORDLEN) return 0;
	int ct = captype(word);

	switch(ct) {
		case HUHCAP:
		case NOCAP:
			rv = check(word); 
			break;

		case INITCAP:
			rv = check(word);
			if (rv == NULL)
			{
				memcpy(wspace,word,(wl+1));
				mkallsmall(wspace);
				rv = check(wspace);
			}
			break;

		case ALLCAP:
			memcpy(wspace,word,(wl+1));
			mkallsmall(wspace);
			rv = check(wspace);
			if (rv == NULL)
			{
				mkinitcap(wspace);
				rv = check(wspace);
				if (rv == NULL) rv = check(word);
			}
			break; 

	}
	if (rv) return 1;
	return 0;
}


char * MySpell::check(const char * word) const
{
	int nLength = strlen(word);
	struct hentry * he = NULL;
	if (pHMgr)
		he = pHMgr->lookup (word);

	if ((he == NULL) && pIgnoreHash)
		he = pIgnoreHash->lookup(word);

	if ((he == NULL) && pAddHash)
		he = pAddHash->lookup(word);

	if ((he == NULL) && pAMgr && pHMgr) {
		// try stripping off affixes */
		he = pAMgr->affix_check(pHMgr, word, nLength);
	}

	if ((he == NULL) && pAMgr && pAddHash && !bMainOnly)
		he = pAMgr->affix_check(pAddHash, word, nLength);

	if (he) return he->word;
	return NULL;
}


int MySpell::suggest(char*** slst, const char * word) const
{
	char wspace[MAXWORDLEN+1];
	if (! pSMgr) return 0;
	int wl = strlen(word);
	int ct = captype(word);
	int ns = 0;
	int j;
	switch(ct) {
		case NOCAP:
			//Try as it is
			ns = pSMgr->suggest(pHMgr, -1, slst, word);
			if (!bMainOnly) ns = pSMgr->suggest(pAddHash, ns, slst, word);
			//Try with init cap for German
			memcpy(wspace, word, (wl+1));
			mkinitcap(wspace);
			ns = pSMgr->suggest(pHMgr, ns, slst, wspace);
			if (!bMainOnly) ns = pSMgr->suggest(pAddHash, ns, slst, wspace);
			break;

		case INITCAP:
			{
				//Try it as it is
				ns = pSMgr->suggest(pHMgr, -1, slst, word);
				if (!bMainOnly) ns = pSMgr->suggest(pAddHash, ns, slst, word);
				//Try it in lower case
				memcpy(wspace, word, (wl+1));
				mkallsmall(wspace);
				const int idxStartLowerCaseSuggestions = ns;
				ns = pSMgr->suggest(pHMgr, ns, slst, wspace);
				if (!bMainOnly) ns = pSMgr->suggest(pAddHash, ns, slst, wspace);
				// - give these ones an initcap again (start of a sentence for example)
				for (j=idxStartLowerCaseSuggestions; j < ns; j++)
				{
					mkinitcap((*slst)[j]);
				}
				break;
			}

		case HUHCAP:
			//Try both - original and all lowercase
			memcpy(wspace,word,(wl+1));
			mkallsmall(wspace);
			ns = pSMgr->suggest(pHMgr, -1, slst, word);
			ns = pSMgr->suggest(pHMgr, ns,slst, wspace);
			if ( !bMainOnly ) {
				ns = pSMgr->suggest(pAddHash, ns, slst, word);
				ns = pSMgr->suggest(pAddHash, ns, slst, wspace);
			}
			break;

		case ALLCAP:
			{
				//Try it as it is
				ns = pSMgr->suggest(pHMgr, -1, slst, word);
				if (!bMainOnly) ns = pSMgr->suggest(pAddHash, ns, slst, word);
				//Try it with other cases
				const int idxStartOtherCaseSuggestions = ns;
				// - lower case
				memcpy(wspace, word, (wl+1));
				mkallsmall(wspace);
				ns = pSMgr->suggest(pHMgr, ns, slst, wspace);
				if (!bMainOnly) ns = pSMgr->suggest(pAddHash, ns, slst, wspace);
				// - init cap
				mkinitcap(wspace);
				ns = pSMgr->suggest(pHMgr, ns, slst, wspace);
				if (!bMainOnly) ns = pSMgr->suggest(pAddHash, ns, slst, wspace);
				// - everything back to allcap
				for (j=idxStartOtherCaseSuggestions; j < ns; j++)
				{
					mkallcap((*slst)[j]);
				}
			}
			break;
	}
	if (ns < 1)
		release_suggest(slst);
	return ns;
}


void MySpell::mkinitcap(char * p) const
{
	if (*p != '\0') *p = toupper((unsigned char)*p);
}


void MySpell::ignore_word(const char *word)
{
	if (pIgnoreHash) {
		pIgnoreHash->add_word(word);
		bModified = true;
	}
}


void MySpell::add_word(const char *word)
{
	if (pAddHash) {
		pAddHash->add_word(word);
		bModified = true;
	}
}


void MySpell::set_personal_dictionary(const char *fileName)
{
	delete [] strPersonalDictionaryPath;
	strPersonalDictionaryPath = strdup(fileName);
}


int MySpell::open_personal_dictionary()
{
	delete pAddHash;
	if (strPersonalDictionaryPath == NULL) {
		// Create an empty dictionary
		pAddHash = new HashMgr;
	} else {
		try {
			pAddHash = new HashMgr(strPersonalDictionaryPath);
		} catch ( ... ) {
			pAddHash = new HashMgr;
			return -1;
		}
	}
	return 0;
}


int MySpell::save_personal_dictionary(const char *fileName)
{
	int retValue = -1;
	if (fileName && pAddHash) {
		retValue = pAddHash->save_tables(fileName);
		if (retValue == 0)
			bModified = false;
	}
	return retValue;
}


int MySpell::open_ignored_words(const char *fileName)
{
	try {
		delete pIgnoreHash;
		pIgnoreHash = new HashMgr(fileName);
	} catch (...) {
		pIgnoreHash = new HashMgr();
		return -1;
	}
	return 0;
}


int MySpell::save_ignored_words(const char *fileName)
{
	int retValue = -1;
	if (fileName && pIgnoreHash) {
		retValue = pIgnoreHash->save_tables(fileName);
		if (retValue == 0)
			bModified = false;
	}
	return retValue;
}