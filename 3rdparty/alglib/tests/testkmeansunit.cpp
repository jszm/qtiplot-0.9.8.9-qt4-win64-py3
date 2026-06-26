
#include <stdafx.h>
#include <stdio.h>
#include "testkmeansunit.h"

static void simpletest1(int nvars,
     int nc,
     int passcount,
     bool& converrors,
     bool& othererrors,
     bool& simpleerrors);
static void restartstest(bool& converrors, bool& restartserrors);
static double rnormal();
static double rsphere(ap::real_2d_array& xy, int n, int i);

bool testkmeans(bool silent)
{
    bool result;
    int nf;
    int maxnf;
    int nc;
    int maxnc;
    int passcount;
    int pass;
    bool waserrors;
    bool converrors;
    bool simpleerrors;
    bool complexerrors;
    bool othererrors;
    bool restartserrors;

    
    //
    // Primary settings
    //
    maxnf = 5;
    maxnc = 5;
    passcount = 10;
    waserrors = false;
    converrors = false;
    othererrors = false;
    simpleerrors = false;
    complexerrors = false;
    restartserrors = false;
    
    //
    //
    //
    for(nf = 1; nf <= maxnf; nf++)
    {
        for(nc = 1; nc <= maxnc; nc++)
        {
            simpletest1(nf, nc, passcount, converrors, othererrors, simpleerrors);
        }
    }
    restartstest(converrors, restartserrors);
    
    //
    // Final report
    //
    waserrors = converrors||othererrors||simpleerrors||complexerrors||restartserrors;
    if( !silent )
    {
        printf("K-MEANS TEST\n");
        printf("TOTAL RESULTS:                           ");
        if( !waserrors )
        {
            printf("OK\n");
        }
        else
        {
            printf("FAILED\n");
        }
        printf("* CONVERGENCE:                           ");
        if( !converrors )
        {
            printf("OK\n");
        }
        else
        {
            printf("FAILED\n");
        }
        printf("* SIMPLE TASKS:                          ");
        if( !simpleerrors )
        {
            printf("OK\n");
        }
        else
        {
            printf("FAILED\n");
        }
        printf("* COMPLEX TASKS:                         ");
        if( !complexerrors )
        {
            printf("OK\n");
        }
        else
        {
            printf("FAILED\n");
        }
        printf("* OTHER PROPERTIES:                      ");
        if( !othererrors )
        {
            printf("OK\n");
        }
        else
        {
            printf("FAILED\n");
        }
        printf("* RESTARTS PROPERTIES:                   ");
        if( !restartserrors )
        {
            printf("OK\n");
        }
        else
        {
            printf("FAILED\n");
        }
        if( waserrors )
        {
            printf("TEST SUMMARY: FAILED\n");
        }
        else
        {
            printf("TEST SUMMARY: PASSED\n");
        }
        printf("\n\n");
    }
    result = !waserrors;
    return result;
}


/*************************************************************************
Simple test 1: ellipsoid in NF-dimensional space.
compare k-means centers with random centers
*************************************************************************/
static void simpletest1(int nvars,
     int nc,
     int passcount,
     bool& converrors,
     bool& othererrors,
     bool& simpleerrors)
{
    int npoints;
    int majoraxis;
    ap::real_2d_array xy;
    ap::real_1d_array tmp;
    double v;
    int i;
    int j;
    int info;
    ap::real_2d_array c;
    ap::integer_1d_array xyc;
    int pass;
    int restarts;
    double ekmeans;
    double erandom;
    double dclosest;
    int cclosest;

    npoints = nc*100;
    restarts = 5;
    passcount = 10;
    tmp.setbounds(0, nvars-1);
    for(pass = 1; pass <= passcount; pass++)
    {
        
        //
        // Fill
        //
        xy.setbounds(0, npoints-1, 0, nvars-1);
        majoraxis = ap::randominteger(nvars);
        for(i = 0; i <= npoints-1; i++)
        {
            rsphere(xy, nvars, i);
            xy(i,majoraxis) = nc*xy(i,majoraxis);
        }
        
        //
        // Test
        //
        kmeansgenerate(xy, npoints, nvars, nc, restarts, info, c, xyc);
        if( info<0 )
        {
            converrors = true;
            return;
        }
        
        //
        // Test that XYC is correct mapping to cluster centers
        //
        for(i = 0; i <= npoints-1; i++)
        {
            cclosest = -1;
            dclosest = ap::maxrealnumber;
            for(j = 0; j <= nc-1; j++)
            {
                ap::vmove(&tmp(0), 1, &xy(i, 0), 1, ap::vlen(0,nvars-1));
                ap::vsub(&tmp(0), 1, &c(0, j), c.getstride(), ap::vlen(0,nvars-1));
                v = ap::vdotproduct(&tmp(0), 1, &tmp(0), 1, ap::vlen(0,nvars-1));
                if( ap::fp_less(v,dclosest) )
                {
                    cclosest = j;
                    dclosest = v;
                }
            }
            if( cclosest!=xyc(i) )
            {
                othererrors = true;
                return;
            }
        }
        
        //
        // Use first NC rows of XY as random centers
        // (XY is totally random, so it is as good as any other choice).
        //
        // Compare potential functions.
        //
        ekmeans = 0;
        for(i = 0; i <= npoints-1; i++)
        {
            ap::vmove(&tmp(0), 1, &xy(i, 0), 1, ap::vlen(0,nvars-1));
            ap::vsub(&tmp(0), 1, &c(0, xyc(i)), c.getstride(), ap::vlen(0,nvars-1));
            v = ap::vdotproduct(&tmp(0), 1, &tmp(0), 1, ap::vlen(0,nvars-1));
            ekmeans = ekmeans+v;
        }
        erandom = 0;
        for(i = 0; i <= npoints-1; i++)
        {
            dclosest = ap::maxrealnumber;
            for(j = 0; j <= nc-1; j++)
            {
                ap::vmove(&tmp(0), 1, &xy(i, 0), 1, ap::vlen(0,nvars-1));
                ap::vsub(&tmp(0), 1, &xy(j, 0), 1, ap::vlen(0,nvars-1));
                v = ap::vdotproduct(&tmp(0), 1, &tmp(0), 1, ap::vlen(0,nvars-1));
                if( ap::fp_less(v,dclosest) )
                {
                    dclosest = v;
                }
            }
            erandom = erandom+v;
        }
        if( ap::fp_less(erandom,ekmeans) )
        {
            simpleerrors = true;
            return;
        }
    }
}


/*************************************************************************
This non-deterministic test checks that Restarts>1 significantly  improves
quality of results.

Subroutine generates random task 3 unit balls in 2D, each with 20  points,
separated by 5 units wide gaps, and solves it  with  Restarts=1  and  with
Restarts=5. Potential functions are compared,  outcome  of  the  trial  is
either 0 or 1 (depending on what is better).

Sequence of 1000 such tasks is  solved.  If  Restarts>1  actually  improve
quality of solution, sum of outcome will be non-binomial.  If  it  doesn't
matter, it will be binomially distributed.

P.S. This test was added after report from Gianluca  Borello  who  noticed
error in the handling of multiple restarts.
*************************************************************************/
static void restartstest(bool& converrors, bool& restartserrors)
{
    int npoints;
    int nvars;
    int nclusters;
    int clustersize;
    int restarts;
    int passcount;
    double sigmathreshold;
    double p;
    double s;
    ap::real_2d_array xy;
    ap::real_2d_array ca;
    ap::real_2d_array cb;
    ap::integer_1d_array xyca;
    ap::integer_1d_array xycb;
    ap::real_1d_array tmp;
    int i;
    int j;
    int info;
    int pass;
    double ea;
    double eb;
    double v;

    restarts = 5;
    passcount = 1000;
    clustersize = 20;
    nclusters = 3;
    nvars = 2;
    npoints = nclusters*clustersize;
    sigmathreshold = 5;
    xy.setlength(npoints, nvars);
    tmp.setlength(nvars);
    p = 0;
    for(pass = 1; pass <= passcount; pass++)
    {
        
        //
        // Fill
        //
        for(i = 0; i <= npoints-1; i++)
        {
            rsphere(xy, nvars, i);
            for(j = 0; j <= nvars-1; j++)
            {
                xy(i,j) = xy(i,j)+double(i)/double(clustersize)*5;
            }
        }
        
        //
        // Test: Restarts=1
        //
        kmeansgenerate(xy, npoints, nvars, nclusters, 1, info, ca, xyca);
        if( info<0 )
        {
            converrors = true;
            return;
        }
        ea = 0;
        for(i = 0; i <= npoints-1; i++)
        {
            ap::vmove(&tmp(0), 1, &xy(i, 0), 1, ap::vlen(0,nvars-1));
            ap::vsub(&tmp(0), 1, &ca(0, xyca(i)), ca.getstride(), ap::vlen(0,nvars-1));
            v = ap::vdotproduct(&tmp(0), 1, &tmp(0), 1, ap::vlen(0,nvars-1));
            ea = ea+v;
        }
        
        //
        // Test: Restarts>1
        //
        kmeansgenerate(xy, npoints, nvars, nclusters, restarts, info, cb, xycb);
        if( info<0 )
        {
            converrors = true;
            return;
        }
        eb = 0;
        for(i = 0; i <= npoints-1; i++)
        {
            ap::vmove(&tmp(0), 1, &xy(i, 0), 1, ap::vlen(0,nvars-1));
            ap::vsub(&tmp(0), 1, &cb(0, xycb(i)), cb.getstride(), ap::vlen(0,nvars-1));
            v = ap::vdotproduct(&tmp(0), 1, &tmp(0), 1, ap::vlen(0,nvars-1));
            eb = eb+v;
        }
        
        //
        // Calculate statistic.
        //
        if( ap::fp_less(ea,eb) )
        {
            p = p+1;
        }
        if( ap::fp_eq(ea,eb) )
        {
            p = p+0.5;
        }
    }
    
    //
    // If Restarts doesn't influence quality of centers found, P must be
    // binomially distributed random value with mean 0.5*PassCount and
    // standard deviation Sqrt(PassCount/4).
    //
    // If Restarts do influence quality of solution, P must be significantly
    // lower than 0.5*PassCount.
    //
    s = (p-0.5*passcount)/sqrt(double(passcount)/double(4));
    restartserrors = restartserrors||ap::fp_greater(s,-sigmathreshold);
}


/*************************************************************************
Random normal number
*************************************************************************/
static double rnormal()
{
    double result;
    double u;
    double v;
    double s;
    double x1;
    double x2;

    while(true)
    {
        u = 2*ap::randomreal()-1;
        v = 2*ap::randomreal()-1;
        s = ap::sqr(u)+ap::sqr(v);
        if( ap::fp_greater(s,0)&&ap::fp_less(s,1) )
        {
            s = sqrt(-2*log(s)/s);
            x1 = u*s;
            x2 = v*s;
            break;
        }
    }
    result = x1;
    return result;
}


/*************************************************************************
Random point from sphere
*************************************************************************/
static double rsphere(ap::real_2d_array& xy, int n, int i)
{
    double result;
    int j;
    double v;

    for(j = 0; j <= n-1; j++)
    {
        xy(i,j) = rnormal();
    }
    v = ap::vdotproduct(&xy(i, 0), 1, &xy(i, 0), 1, ap::vlen(0,n-1));
    v = ap::randomreal()/sqrt(v);
    ap::vmul(&xy(i, 0), 1, ap::vlen(0,n-1), v);
    return result;
}


/*************************************************************************
Silent unit test
*************************************************************************/
bool testkmeansunit_test_silent()
{
    bool result;

    result = testkmeans(true);
    return result;
}


/*************************************************************************
Unit test
*************************************************************************/
bool testkmeansunit_test()
{
    bool result;

    result = testkmeans(false);
    return result;
}




