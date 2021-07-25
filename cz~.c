#include "m_pd.h"
#include <math.h>

enum
{
  T_SAW = 0,
  T_SQUARE,
  T_PULSE,
  T_SINEPULSE,
  T_HALFPULSE,
  T_RES_SAW,
  T_RES_TRIANGLE,
  T_RES_TRAPEZOID
};

#define AC_PI   3.141592653589792
#define AC_2PI  6.283185307179586
#define SSIZE   2048
#define SSIZE_1 2047
#define SH      512

#define AF_SINT(F, B_I, OUT)                                    \
  (F) = ((F) * SSIZE_1);                                        \
  if ((F) > SSIZE_1)                                            \
    (F) -= SSIZE_1;                                             \
  (B_I) = (F);                                                  \
  (F) = (F) - (B_I);                                            \
  (OUT) = (sint[(B_I)+1] - sint[(B_I)]) * (F) + sint[(B_I)];

#define AF_COST(F, B_I, OUT)                                    \
  (F) = ((F) * SSIZE_1) + SH;                                   \
  if ((F) > SSIZE_1)                                            \
    (F) -= SSIZE_1;                                             \
  (B_I) = (F);                                                  \
  (F) = (F) - (B_I);                                            \
  (OUT) = (sint[(B_I)+1] - sint[(B_I)]) * (F) + sint[(B_I)];

static t_class *cz_class;

typedef struct _cz
{
  t_object x_obj;
  int type1;
  int type2;
} t_cz;

t_float sint[SSIZE];

//----------------------------------------------------------------------------//
t_int *cz_perform(t_int *w)
{
  t_cz *x = (t_cz *)(w[1]);
  t_float *in_phase = (t_float *)(w[2]);
  t_float *in_wave = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
  int n = (int)(w[5]);
  t_float f;
  t_float ph;
  t_float phi;
  t_float wave;
  t_float a,b,c;
  int type;
  int type1 = x->type1;
  int type2 = x->type2;
  t_float wv_050;
  t_float a_050;
  t_float b_050;
  t_float wv_res;
  int i;


  while (n--)
    {
      // phase wrap
      ph = *(in_phase++);
      if      (ph > 1.0) {   i = ph;   ph -= i;      }
      else if (ph < 0.0) {   i = ph;   ph = i - ph;  }

      // phase switch
      ph += ph;
      if (ph < 1.0) {  phi = ph;       type = type1;  }
      else          {  phi = ph - 1.0; type = type2;  }
      
      // wave
      wave = *(in_wave++);

      // shaper
      switch(type)
        {
        case T_SQUARE:
          if      (wave < 0.0)   wave = 0.0;
          else if (wave > 0.99)  wave = 0.99;
          wave = wave / (1.0 - wave);
          a = phi + phi + 1.0;
          i = a;
          c = a = a - i;
          b = (1.0 - a) * wave;
          if (a > b) a = b;
          c = c - a;
          if (phi >= 0.5) a = 1.0;
          else            a = 0.0;
          a += c;
          a *= 0.5;
          AF_COST(a, i, f);
          break;
              
          
        case T_PULSE:
          if      (wave < 0.0)   wave = 0.0;
          else if (wave > 0.95)  wave = 0.95;
          wave = wave / (1.0 - wave);
          a = (1.0 - phi) * wave;
          if (phi > a) c = a;
          else         c = phi;
          c = phi - c;
          AF_COST(c, i, f);
          break;
          
          
        case T_SINEPULSE:
          if      (wave < 0.0)   wave = 0.0;
          else if (wave > 0.95)  wave = 0.95;
          wv_050 = 0.5 - (wave * 0.5);
          a_050 = 0.5 - wv_050;
          b = (a_050 / wv_050) * phi;
          c = (1.0 - phi) * (a_050 / (1.0 - wv_050));
          if (b > c) b = c;
          c = phi + b;
          c = c+c;
          if (c > 1.0) c -= 1.0;
          AF_COST(c, i, f);
          break;
          
          
        case T_HALFPULSE:
          if      (wave < 0.0)   wave = 0.0;
          else if (wave > 0.99)  wave = 0.99;
          wv_050 = 0.5 - (wave * 0.5);
          a = (phi - 0.5) / wv_050;
          a = (a * 0.5) + 0.5;
          if (phi < 0.5) b = 1.0;
          else           b = 0.0;
          c = a * (1.0 - b);
          c = (phi * b) + c;
          if (c > 1.0) c = 1.0;
          AF_COST(c, i, f);
          break;
          
          
        case T_RES_SAW:
          if (wave < 0.0) wv_res = 1.0;
          else            wv_res = (wave * 31.0) + 1.0;
          a = (phi * wv_res) + 1.0;
          i = a;
          a = a - i;
          AF_COST(a, i, a);
          a = (a * -0.5) + 0.5;
          b = -1.0 + phi;
          b = b * a;
          b += 0.5;
          f = b+b;
          break;
          
          
        case T_RES_TRIANGLE:
          if (wave < 0.0) wv_res = 1.0;
          else            wv_res = (wave * 31.0) + 1.0;
          a = (phi * wv_res) + 1.0;
          i = a;
          a = a - i;
          AF_COST(a, i, a);
          a = (a * -0.5) + 0.5;
          phi += phi;
          if (phi < 1.0) b = 1.0;
          else           b = 0.0;
          c = (2.0 - phi) * (1.0 - b);
          b = (phi * b) + c; 
          b = b * a;
          b -= 0.5;
          f = 0 - b - b;
          break;
          
          
        case T_RES_TRAPEZOID:
          if (wave < 0.0) wv_res = 1.0;
          else            wv_res = (wave * 31.0) + 1.0;
          a = (phi * wv_res) + 1.0;
          i = a;
          a = a - i;
          AF_COST(a, i, a);
          a = (a * -0.5) + 0.5;
          b = -1.0 + phi;
          if      (b > 0.5)  b = 0.5;
          else if (b < -0.5) b = -0.5;
          b += b;
          b = b * a;
          b += 0.5;
          f = b + b;
          break;
          
          
        default: // T_SAW
          if      (wave < 0.0)   wave = 0.0;
          else if (wave > .999)  wave = 0.999;
          wv_050 = 0.5 - (wave * 0.5);
          a_050 = 0.5 - wv_050;
          b_050 = a_050 / (1.0 - wv_050);
          b = (1.0 - phi) * b_050;
          c = phi * (a_050 / wv_050);
          if (c > b) c = b;
          c += phi;
          AF_COST(c, i, f);
          break;
        }
      
      *(out++) = f;
    }
  
  return (w + 6);
}

//----------------------------------------------------------------------------//
void cz_type1(t_cz *x, t_floatarg f)
{
  x->type1 = f;
}

//----------------------------------------------------------------------------//
void cz_type2(t_cz *x, t_floatarg f)
{
  x->type2 = f;
}

//----------------------------------------------------------------------------//
void cz_dsp(t_cz *x, t_signal **sp)
{
  dsp_add(cz_perform,
          5,
          x,
          sp[0]->s_vec,
          sp[1]->s_vec,
          sp[2]->s_vec,
          sp[0]->s_n);
}

//----------------------------------------------------------------------------//
static void *cz_new(t_symbol *s, int ac, t_atom *av)
{
  t_cz *x = (t_cz *)pd_new(cz_class);
  x->type1 = atom_getfloatarg(0,ac,av);
  x->type2 = atom_getfloatarg(1,ac,av);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  return (void *)x;
  if (s) {};
}

//----------------------------------------------------------------------------//
void cz_calc_sint()
{
  int i;
  for (i=0; i<SSIZE; i++)
    {
      sint[i] = sin( ((t_float)i / SSIZE) * AC_2PI);
    }
}

//----------------------------------------------------------------------------//
void cz_tilde_setup(void)
{
  cz_class = class_new(gensym("cz~"), (t_newmethod)cz_new, 0,
                       sizeof(t_cz),0,A_GIMME,0);
  class_addmethod(cz_class, nullfn, gensym("signal"), 0);
  class_addmethod(cz_class, (t_method)cz_dsp, gensym("dsp"), 0);
  class_addmethod(cz_class, (t_method)cz_type1, gensym("type1"), A_DEFFLOAT, 0);
  class_addmethod(cz_class, (t_method)cz_type2, gensym("type2"), A_DEFFLOAT, 0);
  cz_calc_sint();
}
