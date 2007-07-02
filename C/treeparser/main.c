// Example of a grammar for a trivial tree parser.
// Adapted from Java equaivalent example, by Terence Parr
// Author: Jim Idle - April 2007
// Permission is granted to use this example code in any way you want, so long as
// all the original authors are cited.
//

// set ts=8,sw=4
// Tab size is 8 chars, indet is 4 chars

// Notes: Although all the examples provided are configured to be built
//        by Visual Studio 2005, based on the custom build rules
//        provided in $(ANTLRSRC)/code/antlr/main/runtime/C/vs2005/rulefiles/antlr3.rules
//        there is no reason that this MUST be the case. Provided that you know how
//        to run the antlr tool, then just compile the resulting .c files and this
//	  file together, using say gcc or whatever: gcc *.c -I. -o XXX
//	  The C code is generic and will compile and run on all platforms (please
//        report any warnings or errors to the antlr-interest newsgroup (see www.antlr.org)
//        so that they may be corrected for any platofrm that I have not specifically tested.
//
//	  The project settings such as addinotal library paths and include paths have been set
//        relative to the place where this source code sits on the ANTLR perforce system. You
//        may well need to change the settings to locate the includes and the lib files. UNIX
//        people need -L path/to/antlr/libs -lantlr3c (release mode) or -lantlr3cd (debug)
//
//        Jim Idle (jimi cut-this at idle ws)
//

// You may adopt your own practices by all means, but in general it is best
// to create a single include for your project, that will include the ANTLR3 C
// runtime header files, the generated header files (all of which are safe to include
// multiple times) and your own project related header files. Use <> to include and
// -I on the compile line (which vs2005 now handles, where vs2003 did not).
//
#include    <treeparser.h>

// Main entry point for this example
//
int ANTLR3_CDECL
main	(int argc, char *argv[])
{
    // Now we declare the ANTLR related local variables we need.
    // Note that unless you are convinced you will never need thread safe
    // versions for your project, then you should always create such things
    // as instance variables for each invocation.
    // -------------------

    // Name of the input file. Note that we always use the abstract type pANTLR3_UINT8
    // for ASCII/8 bit strings - the runtime library garauntees that this will be
    // good on all platforms. This is a general rule - always use the ANTLR3 supplied
    // typedefs for pointers/types/etc.
    //
    pANTLR3_UINT8	    fName;

    // The ANTLR3 character input stream, which abstracts the input source such that
    // it is easy to privide inpput from different sources such as files, or 
    // memory strings.
    //
    // For an ASCII/latin-1 memory string use:
    //	    input = antlr3NewAsciiStringInPlaceStream (stringtouse, (ANTLR3_UINT64) length, NULL);
    //
    // For a UCS2 (16 bit) memory string use:
    //	    input = antlr3NewUCS2StringInPlaceStream (stringtouse, (ANTLR3_UINT64) length, NULL);
    //
    // For input from a file, see code below
    //
    // Note that this is essentially a pointer to a structure containing pointers to functions.
    // You can create your own input stream type (copy one of the existing ones) and override any
    // individual function by installing your own pointer after you have created the standard 
    // version.
    //
    pANTLR3_INPUT_STREAM	    input;

    // The lexer is of course generated by ANTLR, and so the lexer type is not upper case.
    // The lexer is supplied with a pANTLR3_INPUT_STREAM from whence it consumes its
    // input and generates a token stream as output.
    //
    pLangLexer			    lxr;

    // The token stream is produced by the ANTLR3 generated lexer. Again it is a structure based
    // API/Object, which you can customise and override methods of as you wish. a Token stream is
    // supplied to the generated parser, and you can write your own token stream and pass this in
    // if you wish.
    //
    pANTLR3_COMMON_TOKEN_STREAM	    tstream;

    // The Lang parser is also generated by ANTLR and accepts a token stream as explained
    // above. The token stream can be any source in fact, so long as it implements the 
    // ANTLR3_TOKEN_SOURCE interface. In this case the parser does not return anything
    // but it can of cousre speficy any kind of return type from the rule you invoke
    // when calling it.
    //
    pLangParser			    psr;

    // The parser produces an AST, which is returned as a member of the return type of
    // the starting rule (any rule can start first of course). This is a generated type
    // based upon the rule we start with.
    //
    LangParser_decl_return	    langAST;


    // The tree nodes are managed by a tree adaptor, which doles
    // out the nodes upon request. You can make your own tree types and adaptors
    // and override the built in versions. See runtime source for details and
    // eventually the wiki entry for the C target.
    //
    pANTLR3_COMMON_TREE_NODE_STREAM	nodes;

    // Finally, when the parser runs, it will produce an AST that can be traversed by the 
    // the tree parser: c.f. LangDumpDecl.g3t
    //
    pLangDumpDecl		    treePsr;

    // Create the input stream based upon the arguement supplied to us on the command line
    // for this example, the input will always default to ./input if there is no explicit
    // argument.
    //
    if (argc < 2 || argv[1] == NULL)
    {
	fName	=(pANTLR3_UINT8)"./input"; // Note in VS2005 debug, working directory must be configured
    }
    else
    {
	fName	= (pANTLR3_UINT8)argv[1];
    }

    // Create the input stream using the supplied file name
    // (Use antlr3AsciiFileStreamNew for UCS2/16bit input).
    //
    input	= antlr3AsciiFileStreamNew(fName);

    // The input will be created succesfully, providing that there is enoguh
    // memory and the file exists etc
    //
    if ( (ANTLR3_UINT64)input < 0 )
    {
	switch((ANTLR3_UINT64)input)
	{
	case    ANTLR3_ERR_NOMEM:

	    fprintf(stderr, "Unable to open file %s due to malloc() failure1\n", (char *)fName);
	    exit(1);
	    break;

	default:

	    fprintf(stderr, "Failed to open file %s - exit with code %d\n", (char *)fName, (ANTLR3_UINT64)input);
	    exit((int)((ANTLR3_UINT64)input));
	    break;
	}
    }

    // Our input stream is now open and all set to go, so we can create a new instance of our
    // lexer and set the lexer input to our input stream:
    //  (file | memory | ?) --> inputstream -> lexer --> tokenstream --> parser ( --> treeparser )?
    //
    lxr	    = LangLexerNew(input);	    // CLexerNew is generated by ANTLR

    // Need to check for errors
    //
    if ( (ANTLR3_UINT64)lxr < 0 )
    {
	switch((ANTLR3_UINT64)lxr)
	{
	case    ANTLR3_ERR_NOMEM:

	    fprintf(stderr, "Unable to create the lexer due to malloc() failure1\n");
	    exit(1);
	    break;

	default:

	    fprintf(stderr, "Failed to create lexer - exit with code %d\n", (ANTLR3_UINT64)lxr);
	    exit((int)((ANTLR3_UINT64)lxr));
	    break;
	}
    }

    // Our lexer is in place, so we can create the token stream from it
    // NB: Nothing happens yet other than the file has been read. We are just 
    // connecting all these things together and they will be invoked when we
    // call the parser rule. ANTLR3_SIZE_HINT can be left at the default usually
    // unless you have a very large token stream/input. Each generated lexer
    // provides a token source interface, which is the second argument to the
    // token stream creator.
    // Note tha even if you implement your own token structure, it will always
    // contain a standard common token within it and this is the pointer that
    // you pass around to everything else. A common token as a pointer within
    // it that should point to your own outer token structure.
    //
    tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, lxr->pLexer->tokSource);

    if ((ANTLR3_UINT64)tstream == ANTLR3_ERR_NOMEM)
    {
	fprintf(stderr, "Out of memory trying to allocate token stream\n");
	exit(ANTLR3_ERR_NOMEM);
    }

    // Finally, now that we have our lexer constructed, we can create the parser
    //
    psr	    = LangParserNew(tstream);  // CParserNew is generated by ANTLR3

    if ((ANTLR3_UINT64)tstream == ANTLR3_ERR_NOMEM)
    {
	fprintf(stderr, "Out of memory trying to allocate parser\n");
	exit(ANTLR3_ERR_NOMEM);
    }

    // We are all ready to go. Though that looked complicated at first glance,
    // I am sure, you will see that in fact most of the code above is dealing
    // with errors and there isn;t really that much to do (isn;t this always the
    // case in C? ;-).
    //
    // So, we now invoke the parser. All elements of ANTLR3 generated C components
    // as well as the ANTLR C runtime library itself are pseudo objects. This means
    // that they are represented as pointers to structures, which contain any
    // instance data they need, and a set of pointers to other interfaces or
    // 'methods'. Note that in general, these few pointers we have created here are
    // the only things you will ever explicitly free() as everythign else is created
    // via factories, that alloacte memory efficiently and free() everything they use
    // automatically when you close the parser/lexer/etc.
    //
    // Note that this means only that the methods are always called via the object
    // pointer and the first argument to any method, is a pointer to the structure itself.
    // It also has the side advantage, if you are using an IDE such as VS2005 taht can do it
    // that when you type ->, you will see a list of all the methods the object supports.
    //
    langAST = psr->decl(psr);

    // If the parser ran correctly, we will have a tree to parse. In general I recommend
    // keeping your own flags as part of the error trapping, but here is how you can
    // work out if there were errors if you are using the generic error messages
    //
    if (psr->pParser->rec->errorCount > 0)
    {
	fprintf(stderr, "The parser returned %d errors, tree walking aborted.\n", psr->pParser->rec->errorCount);

    }
    else
    {
	nodes	= antlr3CommonTreeNodeStreamNewTree(langAST.tree, ANTLR3_SIZE_HINT); // sIZE HINT WILL SOON BE DEPRECATED!!

	// Tree parsers are given a common tree node stream (or your override)
	//
	treePsr	= LangDumpDeclNew(nodes);

	treePsr->decl(treePsr);
	nodes   ->free  (nodes);	    nodes	= NULL;
	treePsr ->free  (treePsr);	    treePsr	= NULL;
    }

    // We did not return anything from this parser rule, so we can finish. It only remains
    // to close down our open objects, in the reverse order we created them
    //
    psr	    ->free  (psr);	    psr		= NULL;
    tstream ->free  (tstream);	    tstream	= NULL;
    lxr	    ->free  (lxr);	    lxr		= NULL;
    input   ->close (input);	    input	= NULL;

    return 0;
}