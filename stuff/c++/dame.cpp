
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <ctime>

#include "dame.h"


static const char* test_keys[] = {
    "code is poetry.",
    "bachelor",
    "b",
    "jar",
    "programming",
    "bach",
    "badge",
    "byron",
    "baby",
    "babbage",
    "ada",
    "adios",
    "adiction",
    "c",
    ""
};


static void basictest()
{
    dame::DoubleArrayTrie trie;
    dame::DoubleArrayTrie::iterator trieit, trieend;

    const char** begin = test_keys;
    const char** end = test_keys + sizeof(test_keys)/sizeof(const char*);
    const char** keyit;

    trie.setDefaultValue(-1);

    // insert all keys
    int num = 0;
    for (keyit=begin; keyit!=end; ++keyit) {
	trieit = trie.insert(*keyit).first;
	if (trieit != trie.end()) {
	    std::cout << *trieit << " inserted." << std::endl;
	    trieit.value() = num++;
	}
    }
    std::cout << std::endl;
    std::cout << sizeof(test_keys)/sizeof(const char*) << " strings inserted." << std::endl;
    std::cout << trie.size() << " keys contained." << std::endl;

    // reorder TAIL
    trie.shrink();

    // show BASE, CHECK, VALUE, and TAIL
    std::cout << std::endl << "Dumping state :" << std::endl;
    trie.dump();

    // dump all keys
    std::cout << std::endl << "Dumping all keys :" << std::endl;
    for (trieit=trie.begin(); trieit!=trie.end(); ++trieit) {
	std::cout << *trieit << std::endl;
	if (std::find(begin, end, *trieit) == end) {
	    std::cout << "!Excessive " << *trieit << std::endl;
	}
    }

    // check that all inserted strings can be searched.
    trieend = trie.end();
    num = 0;
    for (keyit=begin; keyit!=end; ++keyit,++num) {
	trieit = trie.find(*keyit);
	if (trieit == trieend) {
	    std::cout << "!Absence " << *keyit << std::endl;
	} else {
	    if (trieit.value() != num) {
		std::cout << "!Value " << *keyit << std::endl;
	    }
	}
    }

    // common prefix search
    std::cout << std::endl << "Searching keys prefixed by 'ba' :" << std::endl;
    trieit = trie.findPrefixedBy("ba", trieend);
    while (trieit != trieend) {
	std::cout << *trieit << std::endl;
	++trieit;
    }

    // longest prefix search
    std::cout << std::endl << "Searching longest prefix of 'bachelorism' : ";
    trieit = trie.findLongestPrefixOf("bachelorism");
    if (trieit != trie.end()) {
	std::cout << *trieit << std::endl;
    } else {
	std::cout << "(not found)" << std::endl;
    }

    std::cout << std::endl << "Searching longest prefix of 'bachet' : ";
    trieit = trie.findLongestPrefixOf("bachet");
    if (trieit != trie.end()) {
	std::cout << *trieit << std::endl;
    } else {
	std::cout << "(not found)" << std::endl;
    }

    std::cout << std::endl << "Searching longest prefix of 'ja' : ";
    trieit = trie.findLongestPrefixOf("ja");
    if (trieit != trie.end()) {
	std::cout << *trieit << std::endl;
    } else {
	std::cout << "(not found)" << std::endl;
    }
}

static void perftest(const char* filename)
{
    std::ifstream fs(filename);
    std::list<std::string> keys;

    dame::DoubleArrayTrie trie;
    trie.setDefaultValue(-1);

    if (!fs.is_open()) {
	std::cout << "Can't open " << filename << std::endl;
	return;
    }

    // prepare keys
    char buf[1024];
    while (!fs.eof()) {
	fs.getline(buf, sizeof(buf)-1);
	buf[fs.gcount()] = 0;
	if (fs.gcount() > 0) {
	    keys.push_back(buf);
	}
    }

    std::list<std::string>::iterator it, itend;
    itend = keys.end();

    // insertion performance
    clock_t starttime = clock();
    for (it=keys.begin(); it!=itend; ++it) {
	trie.insert(it->c_str());
    }
    clock_t endtime = clock();
    double duration = (double)(endtime-starttime) / (double)CLOCKS_PER_SEC;

    std::cout << "Inserted " << trie.size() << " keys";
    std::cout << " during " << duration << " seconds." << std::endl;

    // search performance
    dame::DoubleArrayTrie::iterator endtrie = trie.end();
    int notfound = 0;
    starttime = clock();
    for (it=keys.begin(); it!=itend; ++it) {
	if (endtrie == trie.find(it->c_str())) {
	    ++notfound;
	}
    }
    endtime = clock();
    duration = (double)(endtime-starttime) / (double)CLOCKS_PER_SEC;

    std::cout << "Searched " << trie.size() << " keys";
    std::cout << " during " << duration << " seconds." << std::endl;
    std::cout << notfound << " keys were not found." << std::endl;
}

static int localmain(int argc, char* argv[])
{
    if (argc <= 1) {
	basictest();
    } else {
	perftest(argv[1]);
    }

    return 0;
}
