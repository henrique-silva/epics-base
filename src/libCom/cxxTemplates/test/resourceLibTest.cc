

#include <assert.h>
#include <time.h>
#include <stdio.h>

#define INSTANCIATE_RES_LIB_STATIC
#include "resourceLib.h"

#ifdef SUNOS4
#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif
#endif

class fred : public uintId, public tsSLNode<fred> {
public:
	fred (const char *pNameIn, unsigned idIn) : 
			pName(pNameIn), uintId(idIn) {}
	void show (unsigned) 
	{
		printf("fred %s\n", pName);
	}
	void destroy()
	{
		// always on stack so noop
	}
private:
	const char * const pName;
};

class jane : public stringId, public tsSLNode<jane> {
public:
	jane (const char *pNameIn) : stringId(pNameIn) {}

	void testTraverse();

	void destroy()
	{
		// always on stack so noop
	}
};

//
// jane::testTraverse()
//
void jane::testTraverse()
{
	printf("Traverse Test\n");
	this->show(10);
}

//
// explicitly instantiates on compilers that support this
//
#if defined(EXPL_TEMPL)
        //
        // From Stroustrups's "The C++ Programming Language"
        // Appendix A: r.14.9
        //
        // This explicitly instantiates the template class's member
        // functions into "templInst.o"
        //
	template class resTable<fred,uintId>;	
	template class resTable<jane,stringId>;	
#endif

main()
{
        unsigned        	i;
        clock_t         	start, finish;
        double          	duration;
        const int       	LOOPS = 50000;
	resTable<fred,uintId>	intTbl;	
	resTable<jane,stringId>	strTbl;	
	fred			fred1("fred1",0x1000a432);
	fred			fred2("fred2",0x0000a432);
	jane			jane1("rrrrrrrrrrrrrrrrrrrrrrrrrr1");
	jane			jane2("rrrrrrrrrrrrrrrrrrrrrrrrrr2");
	fred			*pFred;
	jane			*pJane;
	uintId			uintId1(0x1000a432);
	uintId			uintId2(0x0000a432);
	stringId		strId1("rrrrrrrrrrrrrrrrrrrrrrrrrr1");
	stringId		strId2("rrrrrrrrrrrrrrrrrrrrrrrrrr2");
	int			status;
 
	status = intTbl.init(8);
	if (status) {
		return -1;
	}
 
        status = intTbl.add(fred1);
        assert (!status);
        status = intTbl.add(fred2);
        assert (!status);
 
        start = clock();
        for(i=0; i<LOOPS; i++){
		pFred = intTbl.lookup(uintId1);	
		assert(pFred==&fred1);
		pFred = intTbl.lookup(uintId1);	
		assert(pFred==&fred1);
		pFred = intTbl.lookup(uintId1);	
		assert(pFred==&fred1);
		pFred = intTbl.lookup(uintId1);	
		assert(pFred==&fred1);
		pFred = intTbl.lookup(uintId1);	
		assert(pFred==&fred1);
		pFred = intTbl.lookup(uintId1);	
		assert(pFred==&fred1);
		pFred = intTbl.lookup(uintId1);	
		assert(pFred==&fred1);
		pFred = intTbl.lookup(uintId1);	
		assert(pFred==&fred1);
		pFred = intTbl.lookup(uintId1);	
		assert(pFred==&fred1);
		pFred = intTbl.lookup(uintId2);	
		assert(pFred==&fred2);
        }
        finish = clock();
 
        duration = finish-start;
        duration /= CLOCKS_PER_SEC;
        printf("It took %15.10f total sec for integer hash lookups\n", duration);
        duration /= LOOPS;
        duration /= 10;
	duration *= 1e6;
        printf("It took %15.10f u sec per integer hash lookup\n", duration);
 
	intTbl.show(10u);

	intTbl.remove(uintId1);
	intTbl.remove(uintId2);

	status = strTbl.init(8);
	if (status) {
		return -1;
	}

        status = strTbl.add(jane1);
        assert (!status);
        status = strTbl.add(jane2);
        assert (!status);
 
        start = clock();
        for(i=0; i<LOOPS; i++){
		pJane = strTbl.lookup(strId1);	
		assert(pJane==&jane1);
		pJane = strTbl.lookup(strId1);	
		assert(pJane==&jane1);
		pJane = strTbl.lookup(strId1);	
		assert(pJane==&jane1);
		pJane = strTbl.lookup(strId1);	
		assert(pJane==&jane1);
		pJane = strTbl.lookup(strId1);	
		assert(pJane==&jane1);
		pJane = strTbl.lookup(strId1);	
		assert(pJane==&jane1);
		pJane = strTbl.lookup(strId1);	
		assert(pJane==&jane1);
		pJane = strTbl.lookup(strId1);	
		assert(pJane==&jane1);
		pJane = strTbl.lookup(strId1);	
		assert(pJane==&jane1);
		pJane = strTbl.lookup(strId2);	
		assert(pJane==&jane2);
        }
        finish = clock();

        duration = finish-start;
        duration /= CLOCKS_PER_SEC;
        printf("It took %15.10f total sec for string hash lookups\n", duration);
        duration /= LOOPS;
        duration /= 10;
	duration *= 1e6;
        printf("It took %15.10f u sec per string hash lookup\n", duration);
 
	strTbl.show(10u);
	strTbl.traverse(&jane::testTraverse);

	strTbl.remove(strId1);
	strTbl.remove(strId2);

        return 0;
}

