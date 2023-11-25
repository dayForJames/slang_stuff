

[Differentiable]
float 
f1(float x, float y)
{
    return x * x + 2 * y;
}

[Differentiable]
float 
f2(float x, float y)
{
    return x + 10 + y;
}

[Differentiable]
float 
f3(float x, float y)
{
    return x;
}

__global public __extern_cpp void
computeMain(float **J)
{
    float x = 10.0;
    float y = 4.0;

    let res = fwd_diff(f1)(diffPair(x, 1), diffPair(y, 0));
    //printf("%f", res);
    var dpair_x1 = diffPair(10, 1);
    var dpair_y1 = diffPair(4, 1);

    var bwd_res = bwd_diff(f1)(dpair_x1, dpair_y1, 1);
    //float f1_diff_array[2];

    J[0][0] = dpair_x1.d;
    J[0][1] = dpair_y1.d;

    var dpair_x2 = diffPair(10, 1);
    var dpair_y2 = diffPair(4, 1);
    bwd_res = bwd_diff(f2)(dpair_x2, dpair_y2, 1);
    J[1][0] = dpair_x2.d;
    J[1][1] = dpair_y2.d;

    var dpair_x3 = diffPair(10, 1);
    var dpair_y3 = diffPair(4, 1);
    bwd_res = bwd_diff(f3)(dpair_x3, dpair_y3, 1);
    J[2][0] = dpair_x3.d;
    J[2][1] = dpair_y3.d;

    //return f1_diff_array;
}