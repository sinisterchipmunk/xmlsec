#include "xmlsecrb.h"

VALUE set_id_attribute(VALUE self, VALUE rb_attr_name) {
  VALUE rb_exception_result = Qnil;
  const char* exception_message = NULL;

  xmlNodePtr node = NULL;
  xmlAttrPtr attr = NULL;
  xmlAttrPtr tmp = NULL;
  xmlChar *name = NULL;
  char *idName = NULL;
  char exception_message_buffer[1024] = {'\0'};
  
  Data_Get_Struct(self, xmlNode, node);
  Check_Type(rb_attr_name, T_STRING);
  idName = strndup(RSTRING_PTR(rb_attr_name),
                   RSTRING_LEN(rb_attr_name) + 1);

  // find pointer to id attribute
  attr = xmlHasProp(node, (const xmlChar* )idName);
  if((attr == NULL) || (attr->children == NULL)) {
    rb_exception_result = rb_eRuntimeError;
    exception_message = "Can't find attribute to add register as id";
    goto done;
  }
  
  // get the attribute (id) value
  name = xmlNodeListGetString(node->doc, attr->children, 1);
  if(name == NULL) {
    rb_exception_result = rb_eRuntimeError;
    snprintf(exception_message_buffer, sizeof(exception_message_buffer),
             "Attribute %s has no value", idName);
    exception_message = &exception_message_buffer[0];
    goto done;
  }
  
  // check that we don't have that id already registered
  tmp = xmlGetID(node->doc, name);
  if(tmp != NULL) {
    rb_exception_result = rb_eRuntimeError;
    snprintf(exception_message_buffer, sizeof(exception_message_buffer),
             "Attribute %s is already an ID", idName);
    exception_message = &exception_message_buffer[0];
    goto done;
  }
  
  // finally register id
  xmlAddID(NULL, node->doc, name, attr);

done:
  // and do not forget to cleanup
  if (name) {
    xmlFree(name);
  }

  free(idName);

  if(rb_exception_result != Qnil) {
    rb_raise(rb_exception_result, "%s", exception_message);
  }

  return Qtrue;
}
