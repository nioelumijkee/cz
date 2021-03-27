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
  T_RES_TRAPEZOID,
  T_SAW2
};

#define AC_PI  3.141592653589793
#define AC_2PI 6.283185307179586
#define SSIZE 2048
#define SSIZE_1 2047
#define SH 512

#define AF_SINT(F, B_I, OUT)					\
  (F) = ((F) * SSIZE_1);					\
  if ((F) > SSIZE_1)						\
    (F) -= SSIZE_1;						\
  (B_I) = (F);							\
  (F) = (F) - (B_I);						\
  (OUT) = (sint[(B_I)+1] - sint[(B_I)]) * (F) + sint[(B_I)];

#define AF_COST(F, B_I, OUT)						\
  (F) = ((F) * SSIZE_1) + SH;						\
  if ((F) > SSIZE_1)							\
    (F) -= SSIZE_1;							\
  (B_I) = (F);								\
  (F) = (F) - (B_I);							\
  (OUT) = (sint[(B_I)+1] - sint[(B_I)]) * (F) + sint[(B_I)];

static t_class *cz_class;

typedef struct _cz
{
  t_object x_obj;
  t_float sr; /* const */
  t_float div_fr;
  int type1; /* types */
  int type2;
  t_float ph; /* phasor */
  int hp; /* hi-pass filter */
  t_float hp_z;
} t_cz;

t_float sint[SSIZE];

//----------------------------------------------------------------------------//
t_int *cz_perform(t_int *w)
{
  t_cz *x = (t_cz *)(w[1]);
  t_float *in_freq = (t_float *)(w[2]);
  t_float *in_wave = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
  int n = (int)(w[5]);
  t_float fr;
  t_float f;
  t_float div_fr = x->div_fr;
  t_float ph = x->ph;
  t_float phi;
  t_float wv;
  t_float a,b,c;
  int type;
  int type1 = x->type1;
  int type2 = x->type2;
  int hp = x->hp;
  t_float hp_z = x->hp_z;
  int i;

  while (n--)
    {
      // inc
      fr = *(in_freq++);
      fr = fr * div_fr;
      if      (fr > 0.5)     fr = 0.5;
      else if (fr < 0.00001) fr = 0.00001;


      // wave
      wv = *(in_wave++);


      // phasor
      ph += fr;
      if (ph > 2.0)
	{
	  ph -= 2.0;
	}
      
      // phi
      if (ph < 1.0)
	{
	  phi = ph;
	  type = type1;
	}
      else
	{
	  phi = ph - 1.0;
	  type = type2;
	}
      
      switch(type)
	{
	case T_SQUARE:
	  if      (wv < 0.0)   wv = 0.0;
	  else if (wv > 0.99)  wv = 0.99;
	  wv = wv / (1.0 - wv);
	  a = phi + phi + 1.0;
	  i = a; // wrap
	  c = a = a - i;
	  b = (1.0 - a) * wv;
	  if (a > b) a = b; // min
	  c = c - a;
	  if (phi >= 0.5) a = 1.0;
	  else            a = 0.0;
	  a += c;
	  a *= 0.5;
	  AF_COST(a, i, f);
	  break;
	  
	  
	case T_PULSE:
	  if      (wv < 0.0)   wv = 0.0;
	  else if (wv > 0.99)  wv = 0.99;
	  wv = wv / (1.0 - wv);
	  a = (1.0 - phi) * wv;
	  if (phi > a) c = a; // min
	  else         c = phi;
	  c = phi - c;
	  AF_COST(c, i, f);
	  break;
	  
	  
	case T_SINEPULSE:
	  wv = 0.5 - (wv * 0.5);
	  if      (wv < 0.01)   wv = 0.01;
	  else if (wv > 0.5)    wv = 0.5;
	  a = 0.5 - wv;
	  b = (a / wv) * phi;
	  c = (1.0 - phi) * (a / (1.0 - wv));
	  if (b > c) b = c; // min
	  c = phi + b;
	  c = c+c;
	  if (c > 1.0) c -= 1.0;
	  AF_COST(c, i, f);
	  break;
	  
	  
	case T_HALFPULSE:
	  wv = 0.5 - (wv * 0.5);
	  if      (wv < 0.01)   wv = 0.01;
	  else if (wv > 0.5)    wv = 0.5;
	  a = (phi - 0.5) / wv;
	  a = (a * 0.5) + 0.5;
	  if (phi < 0.5) b = 1.0;
	  else           b = 0.0;
	  c = a * (1.0 - b);
	  c = (phi * b) + c;
	  if (c > 1.0) c = 1.0;
	  AF_COST(c, i, f);
	  break;
	      
	      
	case T_RES_SAW:
	  wv = wv * 32.0;
	  if (wv < 1.0)   wv = 1.0;
	  a = (phi * wv) + 1.0;
	  i = a; // wrap
	  a = a - i;
	  AF_COST(a, i, a);
	  a = (a * -0.5) + 0.5;
	  b = -1.0 + phi;
	  b = b * a;
	  b += 0.5;
	  f = b+b;
	  break;
	  
	  
	case T_RES_TRIANGLE:
	  wv = wv * 32.0;
	  if (wv < 1.0)   wv = 1.0;
	  a = (phi * wv) + 1.0;
	  i = a; // wrap
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
	  wv = wv * 32.0;
	  if (wv < 1.0)   wv = 1.0;
	  a = (phi * wv) + 1.0;
	  i = a; // wrap
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
	  wv = wv * 0.5;
	  wv = 0.5 - wv;
	  if      (wv < 0.01) wv = 0.01;
	  else if (wv > 0.5)  wv = 0.5;
	  a = 0.5 - wv;
	  b = 1.0 - wv;
	  b = (1.0 - phi) * (a / b);
	  c = phi * (a / wv);
	  if (c > b) c = b;
	  c += phi;
	  AF_COST(c, i, f);
	  break;
	}
      
      // hp
      if (hp)
	{
	  hp_z = (f - hp_z) * (fr+fr) + hp_z;
	  f = f - hp_z;
	}
      *(out++) = f;
    }
  // store
  x->ph = ph;
  x->hp_z = hp_z;
  
  return (w + 6);
}

//----------------------------------------------------------------------------//
void cz_calc_constant(t_cz *x)
{
  x->div_fr = 1.0 / x->sr;
}

//----------------------------------------------------------------------------//
void cz_reset(t_cz *x, t_floatarg f)
{
  if      (f < 0.0) f = 0.0;
  else if (f > 1.0) f = 1.0;
  x->ph = f+f;
  x->hp_z = 0.0;
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
void cz_hp(t_cz *x, t_floatarg f)
{
  x->hp = f;
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
  if (sp[0]->s_sr != x->sr)
    {
      x->sr = sp[0]->s_sr;
      cz_reset(x, 0.0);
      cz_calc_constant(x);
    }
}

//----------------------------------------------------------------------------//
static void *cz_new(void)
{
  t_cz *x = (t_cz *)pd_new(cz_class);
  x->sr = 44100;
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  cz_reset(x, 0.0);
  cz_calc_constant(x);
  return (void *)x;
}

//----------------------------------------------------------------------------//
void cz_calc_sint()
{
  int i,j;
  t_float f;
  for (i=0; i<SSIZE; i++)
    {
      /* 12-bit */
      f = sin( ((t_float)i / SSIZE) * AC_2PI);
      j = f * 4096.;
      sint[i] = (t_float)j * 0.000244140625;
    }
}

//----------------------------------------------------------------------------//
void cz_tilde_setup(void)
{
  cz_class = class_new(gensym("cz~"), (t_newmethod)cz_new, 0, sizeof(t_cz),0,0,0);
  class_addmethod(cz_class, nullfn, gensym("signal"), 0);
  class_addmethod(cz_class, (t_method)cz_dsp, gensym("dsp"), 0);
  class_addmethod(cz_class, (t_method)cz_type1, gensym("type1"), A_DEFFLOAT, 0);
  class_addmethod(cz_class, (t_method)cz_type2, gensym("type2"), A_DEFFLOAT, 0);
  class_addmethod(cz_class, (t_method)cz_reset, gensym("reset"), A_DEFFLOAT, 0);
  class_addmethod(cz_class, (t_method)cz_hp, gensym("hp"), A_DEFFLOAT, 0);
  cz_calc_sint();
}
