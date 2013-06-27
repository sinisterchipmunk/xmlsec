#include "xmlsecrb.h"

VALUE set_id_attribute(VALUE self, VALUE rb_attr_name) {
  xmlNodePtr node;
  xmlAttrPtr attr;
  xmlAttrPtr tmp;
  xmlChar *name;
  const xmlChar *idName;
  
  Data_Get_Struct(self, xmlNode, node);
  Check_Type(rb_attr_name, T_STRING);
  idName = (const xmlChar *)RSTRING_PTR(rb_attr_name);

  // find pointer to id attribute
  attr = xmlHasProp(node, idName);
  if((attr == NULL) || (attr->children == NULL)) {
    rb_raise(rb_eRuntimeError, "Can't find attribute to add register as id");
    return Qfalse;
  }
  
  // get the attribute (id) value
  name = xmlNodeListGetString(node->doc, attr->children, 1);
  if(name == NULL) {
    rb_raise(rb_eRuntimeError, "Attribute %s has no value", idName);
    return Qfalse;
  }
  
  // check that we don't have that id already registered
  tmp = xmlGetID(node->doc, name);
  if(tmp != NULL) {
    // rb_raise(rb_eRuntimeError, "Attribute %s is already an ID", idName);
    xmlFree(name);
    return Qfalse;
  }
  
  // finally register id
  xmlAddID(NULL, node->doc, name, attr);

  // and do not forget to cleanup
  xmlFree(name);

  return Qtrue;
}
