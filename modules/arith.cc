#define DECL_OP(t,op,name) inline t t##name(t x, t y) { return x op y; }
#define DECL_OPS(t) \
  DECL_OP(t,+,Add) \
  DECL_OP(t,-,Sub) \
  DECL_OP(t,/,Div) \
  DECL_OP(t,*,Mul)

DECL_OPS(int)
DECL_OPS(short)
DECL_OPS(long)
DECL_OPS(float)
DECL_OPS(double)

#undef DECL_OPS
#undef DECL_OP
