#include "Raster.h"
#include <math.h>

float mag2d(float x,float y) {
    return sqrt(x*x+y*y);
}

static inline int min(int a,int b) {
    return a < b ? a : b;
}
static inline int max(int a,int b) {
    return a > b ? a : b;
}

void fswap(float* a,float* b) {
    float tmp = *a;
    *a = *b;
    *b = tmp;
}

void plotPoint(Context* ct,const Vec3 loc,const Color3 color) {
    int x = loc[0],
        y = loc[1];
    if(x < ct->viewport.x || x >= ct->viewport.x+ct->viewport.width ||
       y < ct->viewport.y || y >= ct->viewport.y+ct->viewport.height) {
        return;
    }

    unsigned index = y*ct->_width+x;
    if(ct->depthEnabled) {
        if(ct->_depth[index] >= loc[2]) {
            return;
        }
        ct->_depth[index] = loc[2];
    }
    Uint8* pixels = ct->surface->pixels;
    unsigned i;
    for(i=0;i<3;++i) {
        pixels[index*3+i] = 255*color[i]+0.5;
    }
}

void rasterPoint(Context* ct,const Varyings* varyings) {
    Color3 color;
    ct->fragShader(ct->uniforms,varyings,color);
    plotPoint(ct,varyings->loc,color);
}

void rasterLine(Context* ct,const Varyings* begin,
                            const Varyings* end) {
    const Varyings* line[] = { begin,end };
    unsigned beginIndex = 0,endIndex = 1;
    int x0 = begin->loc[0],y0 = begin->loc[1],
        x1 = end->loc[0],  y1 = end->loc[1];

    int diffx = x1-x0,diffy = y1-y0;

    Axis axis = (abs(diffx) > abs(diffy) ? AXIS_X : AXIS_Y);

    int startCoord,endCoord;
    if(axis == AXIS_X) {
        startCoord = x0;
        endCoord   = x1;
    } else {
        startCoord = y0;
        endCoord   = y1;
    }

    /*
    if(startCoord == endCoord) {
        return;
    }*/
    
    float step   = 1.0/(endCoord-startCoord);
    float factor = step > 0 ? 0 : 1,
          target = 1-factor;

    Varyings* varyings = createVaryings(begin->numAttributes,
                                        begin->attributes);
    
    startCoord = max(startCoord,ct->viewport.x);
    endCoord   = min(endCoord,  ct->viewport.x+ct->viewport.width);

    int istart = max(min(startCoord,endCoord),ct->viewport.x),
        iend   = min(max(startCoord,endCoord),ct->viewport.x
                                              +ct->viewport.width);

    int i;
    for(i=istart;i<iend;++i) {
        interpolateAlongAxis(varyings,axis,i,begin,end);

        rasterPoint(ct,varyings);
    }
    freeVaryings(varyings);
}

void rasterSpansBetween(Context* ct,const Varyings* a1,
                                    const Varyings* a2,
                                    const Varyings* b1,
                                    const Varyings* b2) {
    float astarty = fmin(a1->loc[1],a2->loc[1]),
          aendy   = fmax(a1->loc[1],a2->loc[1]),
          bstarty = fmin(b1->loc[1],b2->loc[1]),
          bendy   = fmax(b1->loc[1],b2->loc[1]);

    float starty = fmax(astarty,bstarty),
          endy   = fmin(aendy,bendy);
    
    if(starty >= endy) {
        return;
    }

    float adiffy = a2->loc[1]-a1->loc[1],
          bdiffy = b2->loc[1]-b1->loc[1];

    float afactor,bfactor;
    if(adiffy == 0) {
        afactor = 0;
    } else {
        afactor = (starty-a1->loc[1])/adiffy;
    }
    if(bdiffy == 0) {
        bfactor = 0;
    } else {
        bfactor = (starty-b1->loc[1])/bdiffy;
    }

    float astep = 1/adiffy,
          bstep = 1/bdiffy;

    Varyings* a = createVaryings(a1->numAttributes,a1->attributes),
            * b = createVaryings(b1->numAttributes,b1->attributes);

    unsigned y;
    for(y=0;y<=endy-starty;++y) {
        interpolateBetween(a,afactor+y*astep,a1,a2);
        interpolateBetween(b,bfactor+y*bstep,b1,b2);

        rasterLine(ct,a,b);
    }

    freeVaryings(a);
    freeVaryings(b);
}

void rasterTriangle(Context* ct,const Varyings* a,
                                const Varyings* b,
                                const Varyings* c) {
    const Varyings* edges[3][2] = { { a,b },
                                    { b,c },
                                    { a,c } };
    unsigned tallest;
    float tallestLength;
    unsigned i;
    for(i=0;i<3;++i) {
        float length = fabs(edges[i][0]->loc[1]-edges[i][1]->loc[1]);
        if(i == 0 || length > tallestLength) {
            tallest = i;
            tallestLength = length;
        }
    }
    unsigned shorter1 = (tallest+1)%3,
             shorter2 = (tallest+2)%3;

    rasterSpansBetween(ct,edges[tallest][0], edges[tallest][1],
                          edges[shorter1][0],edges[shorter1][1]);
    rasterSpansBetween(ct,edges[tallest][0], edges[tallest][1],
                          edges[shorter2][0],edges[shorter2][1]);
}

