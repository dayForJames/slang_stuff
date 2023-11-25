
[ForwardDifferentiable]
float myFunc(float a, float x)
{
    return a * x * x;
}

float sqr(float x) { return x*x; }

[Differentiable]
float myPow2(float x, int n) {
    float rs = 1;   
    
    
    for (int i = 0; i < n; i++)
        rs *= no_diff sqr(x);
    return rs;
}


void
main()
{
    float x = 10.0;
    float a = 4.0;

    let res = fwd_diff(myFunc)(diffPair(a, 0), diffPair(x, 1));
    printf("%f", res);
}