/*
// Sample from:
// http://garfileo.is-programmer.com/2011/3/28/a-simple-example-for-gobject-introspection.25662.html
*/


#ifndef TEST_SAMPLE_H
#define TEST_SAMPLE_H
  
#include <glib-object.h>
 
#define TEST_TYPE_SAMPLE (test_sample_get_type ())
#define TEST_SAMPLE(object) \
        G_TYPE_CHECK_INSTANCE_CAST ((object), TEST_TYPE_SAMPLE, TestSample)
#define TEST_IS_SAMPLE(object) \
        G_TYPE_CHECK_INSTANCE_TYPE ((object), TEST_TYPE_SAMPLE))
#define TEST_SAMPLE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), TEST_TYPE_SAMPLE, TestSampleClass))
#define TEST_IS_SAMPLE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), TEST_TYPE_SAMPLE))
#define TEST_SAMPLE_GET_CLASS(object) \
        (G_TYPE_INSTANCE_GET_CLASS ((object), TEST_TYPE_SAMPLE, TestSampleClass))
 
 
typedef struct _TestSample TestSample;
struct _TestSample {
    GObject parent;
};
  
typedef struct _TestSampleClass TestSampleClass;
struct _TestSampleClass {
    GObjectClass parent_class;
};
  
GType test_sample_get_type (void);

 
void test_sample_printf (TestSample *self);

/**
 * test_create:
 * Return value: (allow-none) (transfer full):
 */
TestSample* test_create( const char *name, int index );
TestSample* test_sample_create(void);

void test_test( float value );
/**
 * test_test2:
 * @value: (array) : values
 */
int test_test2( float *value, int n );

typedef struct Test Test;
struct Test
{
	int index;
};

/**
 * test_Test_New:
 * Return value: (allow-none) (transfer full):
 */
Test* test_Test_New();
 
#endif
