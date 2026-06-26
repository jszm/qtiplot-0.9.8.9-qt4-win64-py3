
#include <stdafx.h>
#include <stdio.h>
#include "testpsplineunit.h"

static void unsetp2(pspline2interpolant& p);
static void unsetp3(pspline3interpolant& p);
static void unset1d(ap::real_1d_array& x);

bool testpsplineinterpolation(bool silent)
{
    bool result;
    bool waserrors;
    bool p2errors;
    bool p3errors;
    double nonstrictthreshold;
    double threshold;
    int passcount;
    double lstep;
    double h;
    int maxn;
    int periodicity;
    int skind;
    int pkind;
    bool periodic;
    double a;
    double b;
    int n;
    int tmpn;
    int i;
    int k;
    double vx;
    double vy;
    double vz;
    double vx2;
    double vy2;
    double vz2;
    double vdx;
    double vdy;
    double vdz;
    double vdx2;
    double vdy2;
    double vdz2;
    double vd2x;
    double vd2y;
    double vd2z;
    double vd2x2;
    double vd2y2;
    double vd2z2;
    double v0;
    double v1;
    ap::real_1d_array x;
    ap::real_1d_array y;
    ap::real_1d_array z;
    ap::real_1d_array t;
    ap::real_1d_array t2;
    ap::real_1d_array t3;
    ap::real_2d_array xy;
    ap::real_2d_array xyz;
    pspline2interpolant p2;
    pspline3interpolant p3;
    spline1dinterpolant s;

    waserrors = false;
    passcount = 20;
    lstep = 0.005;
    h = 0.00001;
    maxn = 10;
    threshold = 10000*ap::machineepsilon;
    nonstrictthreshold = 0.00001;
    p2errors = false;
    p3errors = false;
    
    //
    // Test basic properties of 2- and 3-dimensional splines:
    // * PSpline2ParameterValues() properties
    // * values at nodes
    // * for periodic splines - periodicity properties
    //
    // Variables used:
    // * N              points count
    // * SKind          spline
    // * PKind          parameterization
    // * Periodicity    whether we have periodic spline or not
    //
    for(n = 2; n <= maxn; n++)
    {
        for(skind = 0; skind <= 2; skind++)
        {
            for(pkind = 0; pkind <= 2; pkind++)
            {
                for(periodicity = 0; periodicity <= 1; periodicity++)
                {
                    periodic = periodicity==1;
                    
                    //
                    // skip unsupported combinations of parameters
                    //
                    if( periodic&&n<3 )
                    {
                        continue;
                    }
                    if( periodic&&skind==0 )
                    {
                        continue;
                    }
                    if( n<5&&skind==0 )
                    {
                        continue;
                    }
                    
                    //
                    // init
                    //
                    xy.setlength(n, 2);
                    xyz.setlength(n, 3);
                    taskgenint1dequidist(double(-1), double(+1), n, t2, x);
                    ap::vmove(&xy(0, 0), xy.getstride(), &x(0), 1, ap::vlen(0,n-1));
                    ap::vmove(&xyz(0, 0), xyz.getstride(), &x(0), 1, ap::vlen(0,n-1));
                    taskgenint1dequidist(double(-1), double(+1), n, t2, y);
                    ap::vmove(&xy(0, 1), xy.getstride(), &y(0), 1, ap::vlen(0,n-1));
                    ap::vmove(&xyz(0, 1), xyz.getstride(), &y(0), 1, ap::vlen(0,n-1));
                    taskgenint1dequidist(double(-1), double(+1), n, t2, z);
                    ap::vmove(&xyz(0, 2), xyz.getstride(), &z(0), 1, ap::vlen(0,n-1));
                    unsetp2(p2);
                    unsetp3(p3);
                    if( periodic )
                    {
                        pspline2buildperiodic(xy, n, skind, pkind, p2);
                        pspline3buildperiodic(xyz, n, skind, pkind, p3);
                    }
                    else
                    {
                        pspline2build(xy, n, skind, pkind, p2);
                        pspline3build(xyz, n, skind, pkind, p3);
                    }
                    
                    //
                    // PSpline2ParameterValues() properties
                    //
                    pspline2parametervalues(p2, tmpn, t2);
                    if( tmpn!=n )
                    {
                        p2errors = true;
                        continue;
                    }
                    pspline3parametervalues(p3, tmpn, t3);
                    if( tmpn!=n )
                    {
                        p3errors = true;
                        continue;
                    }
                    p2errors = p2errors||ap::fp_neq(t2(0),0);
                    p3errors = p3errors||ap::fp_neq(t3(0),0);
                    for(i = 1; i <= n-1; i++)
                    {
                        p2errors = p2errors||ap::fp_less_eq(t2(i),t2(i-1));
                        p3errors = p3errors||ap::fp_less_eq(t3(i),t3(i-1));
                    }
                    if( periodic )
                    {
                        p2errors = p2errors||ap::fp_greater_eq(t2(n-1),1);
                        p3errors = p3errors||ap::fp_greater_eq(t3(n-1),1);
                    }
                    else
                    {
                        p2errors = p2errors||ap::fp_neq(t2(n-1),1);
                        p3errors = p3errors||ap::fp_neq(t3(n-1),1);
                    }
                    
                    //
                    // Now we have parameter values stored at T,
                    // and want to test whether the actully correspond to
                    // points
                    //
                    for(i = 0; i <= n-1; i++)
                    {
                        
                        //
                        // 2-dimensional test
                        //
                        pspline2calc(p2, t2(i), vx, vy);
                        p2errors = p2errors||ap::fp_greater(fabs(vx-x(i)),threshold);
                        p2errors = p2errors||ap::fp_greater(fabs(vy-y(i)),threshold);
                        
                        //
                        // 3-dimensional test
                        //
                        pspline3calc(p3, t3(i), vx, vy, vz);
                        p3errors = p3errors||ap::fp_greater(fabs(vx-x(i)),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vy-y(i)),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vz-z(i)),threshold);
                    }
                    
                    //
                    // Test periodicity (if needed)
                    //
                    if( periodic )
                    {
                        
                        //
                        // periodicity at nodes
                        //
                        for(i = 0; i <= n-1; i++)
                        {
                            
                            //
                            // 2-dimensional test
                            //
                            pspline2calc(p2, t2(i)+ap::randominteger(10)-5, vx, vy);
                            p2errors = p2errors||ap::fp_greater(fabs(vx-x(i)),threshold);
                            p2errors = p2errors||ap::fp_greater(fabs(vy-y(i)),threshold);
                            pspline2diff(p2, t2(i)+ap::randominteger(10)-5, vx, vdx, vy, vdy);
                            p2errors = p2errors||ap::fp_greater(fabs(vx-x(i)),threshold);
                            p2errors = p2errors||ap::fp_greater(fabs(vy-y(i)),threshold);
                            pspline2diff2(p2, t2(i)+ap::randominteger(10)-5, vx, vdx, vd2x, vy, vdy, vd2y);
                            p2errors = p2errors||ap::fp_greater(fabs(vx-x(i)),threshold);
                            p2errors = p2errors||ap::fp_greater(fabs(vy-y(i)),threshold);
                            
                            //
                            // 3-dimensional test
                            //
                            pspline3calc(p3, t3(i)+ap::randominteger(10)-5, vx, vy, vz);
                            p3errors = p3errors||ap::fp_greater(fabs(vx-x(i)),threshold);
                            p3errors = p3errors||ap::fp_greater(fabs(vy-y(i)),threshold);
                            p3errors = p3errors||ap::fp_greater(fabs(vz-z(i)),threshold);
                            pspline3diff(p3, t3(i)+ap::randominteger(10)-5, vx, vdx, vy, vdy, vz, vdz);
                            p3errors = p3errors||ap::fp_greater(fabs(vx-x(i)),threshold);
                            p3errors = p3errors||ap::fp_greater(fabs(vy-y(i)),threshold);
                            p3errors = p3errors||ap::fp_greater(fabs(vz-z(i)),threshold);
                            pspline3diff2(p3, t3(i)+ap::randominteger(10)-5, vx, vdx, vd2x, vy, vdy, vd2y, vz, vdz, vd2z);
                            p3errors = p3errors||ap::fp_greater(fabs(vx-x(i)),threshold);
                            p3errors = p3errors||ap::fp_greater(fabs(vy-y(i)),threshold);
                            p3errors = p3errors||ap::fp_greater(fabs(vz-z(i)),threshold);
                        }
                        
                        //
                        // periodicity between nodes
                        //
                        v0 = ap::randomreal();
                        pspline2calc(p2, v0, vx, vy);
                        pspline2calc(p2, v0+ap::randominteger(10)-5, vx2, vy2);
                        p2errors = p2errors||ap::fp_greater(fabs(vx-vx2),threshold);
                        p2errors = p2errors||ap::fp_greater(fabs(vy-vy2),threshold);
                        pspline3calc(p3, v0, vx, vy, vz);
                        pspline3calc(p3, v0+ap::randominteger(10)-5, vx2, vy2, vz2);
                        p3errors = p3errors||ap::fp_greater(fabs(vx-vx2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vy-vy2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vz-vz2),threshold);
                        
                        //
                        // near-boundary test for continuity of function values and derivatives:
                        // 2-dimensional curve
                        //
                        ap::ap_error::make_assertion(skind==1||skind==2, "TEST: unexpected spline type!");
                        v0 = 100*ap::machineepsilon;
                        v1 = 1-v0;
                        pspline2calc(p2, v0, vx, vy);
                        pspline2calc(p2, v1, vx2, vy2);
                        p2errors = p2errors||ap::fp_greater(fabs(vx-vx2),threshold);
                        p2errors = p2errors||ap::fp_greater(fabs(vy-vy2),threshold);
                        pspline2diff(p2, v0, vx, vdx, vy, vdy);
                        pspline2diff(p2, v1, vx2, vdx2, vy2, vdy2);
                        p2errors = p2errors||ap::fp_greater(fabs(vx-vx2),threshold);
                        p2errors = p2errors||ap::fp_greater(fabs(vy-vy2),threshold);
                        p2errors = p2errors||ap::fp_greater(fabs(vdx-vdx2),nonstrictthreshold);
                        p2errors = p2errors||ap::fp_greater(fabs(vdy-vdy2),nonstrictthreshold);
                        pspline2diff2(p2, v0, vx, vdx, vd2x, vy, vdy, vd2y);
                        pspline2diff2(p2, v1, vx2, vdx2, vd2x2, vy2, vdy2, vd2y2);
                        p2errors = p2errors||ap::fp_greater(fabs(vx-vx2),threshold);
                        p2errors = p2errors||ap::fp_greater(fabs(vy-vy2),threshold);
                        p2errors = p2errors||ap::fp_greater(fabs(vdx-vdx2),nonstrictthreshold);
                        p2errors = p2errors||ap::fp_greater(fabs(vdy-vdy2),nonstrictthreshold);
                        if( skind==2 )
                        {
                            
                            //
                            // second derivative test only for cubic splines
                            //
                            p2errors = p2errors||ap::fp_greater(fabs(vd2x-vd2x2),nonstrictthreshold);
                            p2errors = p2errors||ap::fp_greater(fabs(vd2y-vd2y2),nonstrictthreshold);
                        }
                        
                        //
                        // near-boundary test for continuity of function values and derivatives:
                        // 3-dimensional curve
                        //
                        ap::ap_error::make_assertion(skind==1||skind==2, "TEST: unexpected spline type!");
                        v0 = 100*ap::machineepsilon;
                        v1 = 1-v0;
                        pspline3calc(p3, v0, vx, vy, vz);
                        pspline3calc(p3, v1, vx2, vy2, vz2);
                        p3errors = p3errors||ap::fp_greater(fabs(vx-vx2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vy-vy2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vz-vz2),threshold);
                        pspline3diff(p3, v0, vx, vdx, vy, vdy, vz, vdz);
                        pspline3diff(p3, v1, vx2, vdx2, vy2, vdy2, vz2, vdz2);
                        p3errors = p3errors||ap::fp_greater(fabs(vx-vx2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vy-vy2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vz-vz2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vdx-vdx2),nonstrictthreshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vdy-vdy2),nonstrictthreshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vdz-vdz2),nonstrictthreshold);
                        pspline3diff2(p3, v0, vx, vdx, vd2x, vy, vdy, vd2y, vz, vdz, vd2z);
                        pspline3diff2(p3, v1, vx2, vdx2, vd2x2, vy2, vdy2, vd2y2, vz2, vdz2, vd2z2);
                        p3errors = p3errors||ap::fp_greater(fabs(vx-vx2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vy-vy2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vz-vz2),threshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vdx-vdx2),nonstrictthreshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vdy-vdy2),nonstrictthreshold);
                        p3errors = p3errors||ap::fp_greater(fabs(vdz-vdz2),nonstrictthreshold);
                        if( skind==2 )
                        {
                            
                            //
                            // second derivative test only for cubic splines
                            //
                            p3errors = p3errors||ap::fp_greater(fabs(vd2x-vd2x2),nonstrictthreshold);
                            p3errors = p3errors||ap::fp_greater(fabs(vd2y-vd2y2),nonstrictthreshold);
                            p3errors = p3errors||ap::fp_greater(fabs(vd2z-vd2z2),nonstrictthreshold);
                        }
                    }
                }
            }
        }
    }
    
    //
    // Test differentiation, tangents, calculation between nodes.
    //
    // Because differentiation is done in parameterization/spline/periodicity
    // oblivious manner, we don't have to test all possible combinations
    // of spline types and parameterizations.
    //
    // Actually we test special combination with properties which allow us
    // to easily solve this problem:
    // * 2 (3) variables
    // * first variable is sampled from equidistant grid on [0,1]
    // * other variables are random
    // * uniform parameterization is used
    // * periodicity - none
    // * spline type - any (we use cubic splines)
    // Same problem allows us to test calculation BETWEEN nodes.
    //
    for(n = 2; n <= maxn; n++)
    {
        
        //
        // init
        //
        xy.setlength(n, 2);
        xyz.setlength(n, 3);
        taskgenint1dequidist(double(0), double(+1), n, t, x);
        ap::vmove(&xy(0, 0), xy.getstride(), &x(0), 1, ap::vlen(0,n-1));
        ap::vmove(&xyz(0, 0), xyz.getstride(), &x(0), 1, ap::vlen(0,n-1));
        taskgenint1dequidist(double(0), double(+1), n, t, y);
        ap::vmove(&xy(0, 1), xy.getstride(), &y(0), 1, ap::vlen(0,n-1));
        ap::vmove(&xyz(0, 1), xyz.getstride(), &y(0), 1, ap::vlen(0,n-1));
        taskgenint1dequidist(double(0), double(+1), n, t, z);
        ap::vmove(&xyz(0, 2), xyz.getstride(), &z(0), 1, ap::vlen(0,n-1));
        unsetp2(p2);
        unsetp3(p3);
        pspline2build(xy, n, 2, 0, p2);
        pspline3build(xyz, n, 2, 0, p3);
        
        //
        // Test 2D/3D spline:
        // * build non-parametric cubic spline from T and X/Y
        // * calculate its value and derivatives at V0
        // * compare with Spline2Calc/Spline2Diff/Spline2Diff2
        // Because of task properties both variants should
        // return same answer.
        //
        v0 = ap::randomreal();
        spline1dbuildcubic(t, x, n, 0, 0.0, 0, 0.0, s);
        spline1ddiff(s, v0, vx2, vdx2, vd2x2);
        spline1dbuildcubic(t, y, n, 0, 0.0, 0, 0.0, s);
        spline1ddiff(s, v0, vy2, vdy2, vd2y2);
        spline1dbuildcubic(t, z, n, 0, 0.0, 0, 0.0, s);
        spline1ddiff(s, v0, vz2, vdz2, vd2z2);
        
        //
        // 2D test
        //
        pspline2calc(p2, v0, vx, vy);
        p2errors = p2errors||ap::fp_greater(fabs(vx-vx2),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vy-vy2),threshold);
        pspline2diff(p2, v0, vx, vdx, vy, vdy);
        p2errors = p2errors||ap::fp_greater(fabs(vx-vx2),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vy-vy2),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vdx-vdx2),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vdy-vdy2),threshold);
        pspline2diff2(p2, v0, vx, vdx, vd2x, vy, vdy, vd2y);
        p2errors = p2errors||ap::fp_greater(fabs(vx-vx2),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vy-vy2),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vdx-vdx2),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vdy-vdy2),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vd2x-vd2x2),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vd2y-vd2y2),threshold);
        
        //
        // 3D test
        //
        pspline3calc(p3, v0, vx, vy, vz);
        p3errors = p3errors||ap::fp_greater(fabs(vx-vx2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vy-vy2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vz-vz2),threshold);
        pspline3diff(p3, v0, vx, vdx, vy, vdy, vz, vdz);
        p3errors = p3errors||ap::fp_greater(fabs(vx-vx2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vy-vy2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vz-vz2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vdx-vdx2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vdy-vdy2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vdz-vdz2),threshold);
        pspline3diff2(p3, v0, vx, vdx, vd2x, vy, vdy, vd2y, vz, vdz, vd2z);
        p3errors = p3errors||ap::fp_greater(fabs(vx-vx2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vy-vy2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vz-vz2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vdx-vdx2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vdy-vdy2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vdz-vdz2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vd2x-vd2x2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vd2y-vd2y2),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vd2z-vd2z2),threshold);
        
        //
        // Test tangents for 2D/3D
        //
        pspline2tangent(p2, v0, vx, vy);
        p2errors = p2errors||ap::fp_greater(fabs(vx-vdx2/safepythag2(vdx2, vdy2)),threshold);
        p2errors = p2errors||ap::fp_greater(fabs(vy-vdy2/safepythag2(vdx2, vdy2)),threshold);
        pspline3tangent(p3, v0, vx, vy, vz);
        p3errors = p3errors||ap::fp_greater(fabs(vx-vdx2/safepythag3(vdx2, vdy2, vdz2)),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vy-vdy2/safepythag3(vdx2, vdy2, vdz2)),threshold);
        p3errors = p3errors||ap::fp_greater(fabs(vz-vdz2/safepythag3(vdx2, vdy2, vdz2)),threshold);
    }
    
    //
    // Arc length test.
    //
    // Simple problem with easy solution (points on a straight line with
    // uniform parameterization).
    //
    for(n = 2; n <= maxn; n++)
    {
        xy.setlength(n, 2);
        xyz.setlength(n, 3);
        for(i = 0; i <= n-1; i++)
        {
            xy(i,0) = i;
            xy(i,1) = i;
            xyz(i,0) = i;
            xyz(i,1) = i;
            xyz(i,2) = i;
        }
        pspline2build(xy, n, 1, 0, p2);
        pspline3build(xyz, n, 1, 0, p3);
        a = ap::randomreal();
        b = ap::randomreal();
        p2errors = p2errors||ap::fp_greater(fabs(pspline2arclength(p2, a, b)-(b-a)*sqrt(double(2))*(n-1)),nonstrictthreshold);
        p3errors = p3errors||ap::fp_greater(fabs(pspline3arclength(p3, a, b)-(b-a)*sqrt(double(3))*(n-1)),nonstrictthreshold);
    }
    
    //
    // report
    //
    waserrors = p2errors||p3errors;
    if( !silent )
    {
        printf("TESTING SPLINE INTERPOLATION\n");
        
        //
        // Normal tests
        //
        printf("2D TEST:                                 ");
        if( p2errors )
        {
            printf("FAILED\n");
        }
        else
        {
            printf("OK\n");
        }
        printf("3D TEST:                                 ");
        if( p3errors )
        {
            printf("FAILED\n");
        }
        else
        {
            printf("OK\n");
        }
        if( waserrors )
        {
            printf("TEST FAILED\n");
        }
        else
        {
            printf("TEST PASSED\n");
        }
        printf("\n\n");
    }
    
    //
    // end
    //
    result = !waserrors;
    return result;
}


/*************************************************************************
Unset spline, i.e. initialize it with random garbage
*************************************************************************/
static void unsetp2(pspline2interpolant& p)
{
    ap::real_2d_array xy;

    xy.setlength(2, 2);
    xy(0,0) = -1;
    xy(0,1) = -1;
    xy(1,0) = +1;
    xy(1,1) = +1;
    pspline2build(xy, 2, 1, 0, p);
}


/*************************************************************************
Unset spline, i.e. initialize it with random garbage
*************************************************************************/
static void unsetp3(pspline3interpolant& p)
{
    ap::real_2d_array xy;

    xy.setlength(2, 3);
    xy(0,0) = -1;
    xy(0,1) = -1;
    xy(0,2) = -1;
    xy(1,0) = +1;
    xy(1,1) = +1;
    xy(1,2) = +1;
    pspline3build(xy, 2, 1, 0, p);
}


/*************************************************************************
Unsets real vector
*************************************************************************/
static void unset1d(ap::real_1d_array& x)
{

    x.setlength(1);
    x(0) = 2*ap::randomreal()-1;
}


/*************************************************************************
Silent unit test
*************************************************************************/
bool testpsplineunit_test_silent()
{
    bool result;

    result = testpsplineinterpolation(true);
    return result;
}


/*************************************************************************
Unit test
*************************************************************************/
bool testpsplineunit_test()
{
    bool result;

    result = testpsplineinterpolation(false);
    return result;
}




