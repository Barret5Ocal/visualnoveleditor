void Direct3dPerspectiveFovLH(m4 *Out, float Fovy, float Aspect, float zn, float zf)
{
    float yScale = 1/gb_tan(Fovy/2);
    float xScale = yScale / Aspect;
    Out->x.x = xScale;
    Out->x.y = 0;
    Out->x.z = 0;
    Out->x.w = 0;
    
    Out->y.x = 0;
    Out->y.y = yScale;
    Out->y.z = 0;
    Out->y.w = 0;
    
    Out->z.x = 0;
    Out->z.y = 0;
    Out->z.z = zf/(zf-zn);
    Out->z.w = 1;
    
    Out->w.x = 0;
    Out->w.y = 0;
    Out->w.z = -zn*zf/(zf-zn);
    Out->w.w = 0;
}

void Direct3dLookAtLH(m4 *Out, v3 Eye, v3 At, v3 Up)
{
    v3 zaxis;
    gb_vec3_norm(&zaxis, (At - Eye));
    
    v3 xaxis;
    v3 s; 
    gb_vec3_cross(&s, Up, zaxis);
    gb_vec3_norm(&xaxis, s);
    
    v3 yaxis; 
    gb_vec3_cross(&yaxis, zaxis, xaxis);
    
    Out->x.x = xaxis.x;
    Out->x.y = yaxis.x;
    Out->x.z = zaxis.x;
    Out->x.w = 0;
    
    Out->y.x = xaxis.y;
    Out->y.y = yaxis.y;
    Out->y.z = zaxis.y;
    Out->y.w = 0;
    
    Out->z.x = xaxis.z;
    Out->z.y = yaxis.z;
    Out->z.z = zaxis.z;
    Out->z.w = 0;
    
    Out->w.x = -gb_vec3_dot(xaxis, Eye);
    Out->w.y = -gb_vec3_dot(yaxis, Eye);
    Out->w.z = -gb_vec3_dot(zaxis, Eye);
    Out->w.w = 1;
}