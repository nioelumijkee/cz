#include "m_pd.h"
#include <math.h>
/* #include "include/constant.h" */
/* #include "include/math.h" */
/* #include "include/fil.h" */
/* #include "include/conversion.h" */

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

#define AC_PI  3.141592653589793
#define AC_2PI 6.283185307179586
#define SSIZE 2048
#define SSIZE_1 2047

#define AF_SINT(F, B_I, OUT)					\
  (F) = (F) * SSIZE_1;						\
  if ((F) > SSIZE_1)						\
    (F) -= SSIZE_1;						\
  (B_I) = (F);							\
  (F) = (F) - (B_I);						\
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
  t_float f;
  t_float div_fr = x->div_fr;
  t_float ph = x->ph;
  t_float phi;
  t_float wv;
  t_float a,b,c;
  int type;
  int type1 = x->type1;
  int type2 = x->type2;
  int i;

  while (n--)
    {
      // phasor 0 ... 2
      f = *(in_freq++) * div_fr; // inc
      ph += f;
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

      // wave
      wv = *(in_wave++);


      switch(type)
	{
	case T_SQUARE:
	  f = 0.0;
	  break;
	default: // T_SAW
	  wv = wv * 0.5;
	  wv = 0.5 - wv;
	  if      (wv < 0.01) wv = 0.01;
	  else if (wv > 0.5)  wv = 0.5;
	  a = 0.5 - wv;
	  b = 1.0 - wv;
	  b = (1.0 - phi) * (a / (1.0 - wv));
	  c = phi * (a / wv);
	  if (c > b) c = b;
	  c += phi;
	  // sint
	  AF_SINT(c, i, f);
	  break;
	}
      

      *(out++) = f;
    }
  // store
  x->ph = ph;

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
  int i;
  for (i=0; i<SSIZE; i++)
    {
      sint[i] = cos( ((t_float)i / SSIZE) * AC_2PI);
      post("%d = %f",i,sint[i]);
    }
}

//----------------------------------------------------------------------------//
void cz_tilde_setup(void)
{
  cz_class = class_new(gensym("cz~"), (t_newmethod)cz_new, 0, sizeof(t_cz), 0, A_GIMME, 0);
  class_addmethod(cz_class, nullfn, gensym("signal"), 0);
  class_addmethod(cz_class, (t_method)cz_dsp, gensym("dsp"), 0);
  class_addmethod(cz_class, (t_method)cz_type1, gensym("type1"), A_DEFFLOAT, 0);
  class_addmethod(cz_class, (t_method)cz_type2, gensym("type2"), A_DEFFLOAT, 0);
  class_addmethod(cz_class, (t_method)cz_reset, gensym("reset"), A_DEFFLOAT, 0);
  cz_calc_sint();
}
