
#include <stdlib.h>
#include <stdio.h>
#include <glib/gprintf.h>
#include "test-sample.h"
  
G_DEFINE_TYPE (TestSample, test_sample, G_TYPE_OBJECT);
 
#define TEST_SAMPLE_GET_PRIVATE(object) (\
        G_TYPE_INSTANCE_GET_PRIVATE ((object), TEST_TYPE_SAMPLE, TestSamplePrivate))
  
typedef struct _TestSamplePrivate TestSamplePrivate;
struct _TestSamplePrivate
{
	GString *name;
	guint    index;
};
 
enum PROPERTY_SAMPLE
{
	PROPERTY_0,
	PROPERTY_NAME,
	PROPERTY_INDEX,
	N_PROPERTIES
};
 
static void test_sample_set_property
(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{      
	TestSample *self = TEST_SAMPLE (object);
	TestSamplePrivate *priv = TEST_SAMPLE_GET_PRIVATE (self);

	switch (property_id) {
	case PROPERTY_NAME:
		if (priv->name) g_string_free (priv->name, TRUE);
		priv->name = g_string_new (g_value_get_string (value));
		break;
	case PROPERTY_INDEX:
		priv->index = g_value_get_uint (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}
  
static void test_sample_get_property
(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	TestSample *self = TEST_SAMPLE (object);
	TestSamplePrivate *priv = TEST_SAMPLE_GET_PRIVATE (self);
	GString *similar = NULL;

	switch (property_id) {
	case PROPERTY_NAME:
		g_value_set_string (value, priv->name->str);
		break;
	case PROPERTY_INDEX:
		g_value_set_uint (value, priv->index);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}
 
static void
test_sample_init (TestSample *self)
{
}
 
static void
test_sample_class_init (TestSampleClass *klass)
{
	g_type_class_add_private (klass, sizeof (TestSamplePrivate));

	GObjectClass *base_class = G_OBJECT_CLASS (klass);
	base_class->set_property = test_sample_set_property;
	base_class->get_property = test_sample_get_property;
	GParamSpec *properties[N_PROPERTIES] = {NULL,};
	properties[PROPERTY_NAME] =
		g_param_spec_string ("name", 
				"Name",
				"Name",
				NULL,
				G_PARAM_READWRITE);
	properties[PROPERTY_INDEX] =
		g_param_spec_uint ("index",
				"Index",
				"Index",
				0,
				G_MAXUINT,
				0,
				G_PARAM_READWRITE);
	g_object_class_install_properties (base_class, N_PROPERTIES, properties);
}

TestSample* test_create( const char *name, int index )
{
	printf( "test_create: %s\n", name );
	return g_object_new( TEST_TYPE_SAMPLE, "name", name, "index", index, NULL );
}
TestSample* test_sample_create(void)
{
	printf( "test_sample_create\n" );
	return g_object_new( TEST_TYPE_SAMPLE, NULL );
}
 
void
test_sample_printf (TestSample *self)
{
	gchar *name;
	guint index;

	printf( "test_sample_printf: %p\n", self );

	g_object_get (G_OBJECT (self), "name", &name, "index", &index, NULL);

	g_printf ("    Name: %s\n     Index: %d\n", name, index);

	g_free (name);
}
void test_test( float value )
{
	printf( "test_test: %f\n", value );
}
int test_test2( float *value, int n )
{
	int i;
	for(i=0; i<n; ++i) printf( "%3i: %f\n", i, value[i] );
	return 100 * n;
}


Test* test_Test_New()
{
	Test *self = (Test*) malloc( sizeof(Test) );
	return self;
}
