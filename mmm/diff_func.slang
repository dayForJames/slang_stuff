

//  Example 1

[Differentiable]
__global public __extern_cpp float 
f1(float x, float y)
{
    return x * x + 2 * y;
}

[Differentiable]
__global public __extern_cpp float 
f2(float x, float y)
{
    return x + 10 + y;
}

[Differentiable]
__global public __extern_cpp float 
f3(float x, float y)
{
    return x;
}

__global public __extern_cpp void
computeMain(float **J)
{
    var dpair_x1 = diffPair(10, 1);
    var dpair_y1 = diffPair(4, 1);
    float arr[3] = { 1, 2, 3 };
    float arr_diff_type[3] = { 1, 1, 1 };
    var dp = diffPair(arr, arr_diff_type);

    var bwd_res = bwd_diff(f1)(dpair_x1, dpair_y1, 1);

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
}



// Example 2

[Differentiable]
float 
a(float3 x)
{
    return x[0] * x[0] + 2 * x[1];
}

[Differentiable]
float
b(float3 x)
{
    return x[0] + 10 + x[1];
}

[Differentiable]
float
c(float3 x)
{
    return x[0];
}

__global public __extern_cpp void
f(float3 x, float** res)
{
    float3 diff_variables_types = { 1, 1, 1 };
    var diff_p_arr = diffPair(x, diff_variables_types);

    var ans1 = bwd_diff(a)(diff_p_arr, 1);
    res[0][0] = diff_p_arr.d[0];
    res[0][1] = diff_p_arr.d[1];
    res[0][2] = diff_p_arr.d[2];

    var ans2 = bwd_diff(b)(diff_p_arr, 1);
    res[1][0] = diff_p_arr.d[0];
    res[1][1] = diff_p_arr.d[1];
    res[1][2] = diff_p_arr.d[2];

    var ans3 = bwd_diff(c)(diff_p_arr, 1);
    res[2][0] = diff_p_arr.d[0];
    res[2][1] = diff_p_arr.d[1];
    res[2][2] = diff_p_arr.d[2];
}