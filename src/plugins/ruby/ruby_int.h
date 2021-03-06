#include "NWNStructures.h"
#include "hook.h"
#include "ruby.h"
#include "nwscript.h"

#define RUBY_RETVAL "nwnx_retval"

#define OBJECT_INVALID 0x7F000000
//temporary
#define OBJECT_SELF 0x7F000000

#ifdef __cplusplus
extern "C" {
#endif

extern VALUE g_cLocationClass;
extern VALUE g_cEffectClass;
extern VALUE g_cItemPropertyClass;
extern VALUE g_cVectorClass;

extern VALUE g_OBJECT_SELF;

VALUE RubyInt_InitNWScript();
void RubyInt_DefineConstants();

static VALUE vector_get_x(VALUE self);
static VALUE vector_get_y(VALUE self);
static VALUE vector_get_z(VALUE self);
static VALUE location_get_x(VALUE self);
static VALUE location_get_y(VALUE self);
static VALUE location_get_z(VALUE self);
static VALUE location_get_area_id(VALUE self);
static VALUE location_get_facing(VALUE self);
	
#ifdef __cplusplus
}
#endif

