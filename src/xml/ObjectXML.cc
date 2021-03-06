/* -------------------------------------------------------------------------- */
/* Copyright 2002-2012, OpenNebula Project Leads (OpenNebula.org)             */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may    */
/* not use this file except in compliance with the License. You may obtain    */
/* a copy of the License at                                                   */
/*                                                                            */
/* http://www.apache.org/licenses/LICENSE-2.0                                 */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/* -------------------------------------------------------------------------- */

#include <ObjectXML.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ObjectXML::ObjectXML(const string &xml_doc):xml(0),ctx(0)
{
    try
    {
        xml_parse(xml_doc);
    }
    catch(runtime_error& re)
    {
        throw;
    }
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ObjectXML::ObjectXML(const xmlNodePtr node):xml(0),ctx(0)
{
    xml = xmlNewDoc(reinterpret_cast<const xmlChar *>("1.0"));

    if (xml == 0)
    {
        throw("Error allocating XML Document");
    }

    ctx = xmlXPathNewContext(xml);

    if (ctx == 0)
    {
        xmlFreeDoc(xml);
        throw("Unable to create new XPath context");
    }

    xmlNodePtr root_node = xmlDocCopyNode(node,xml,1);

    if (root_node == 0)
    {
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(xml);
        throw("Unable to allocate node");
    }

    xmlDocSetRootElement(xml, root_node);
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ObjectXML::~ObjectXML()
{
    if (xml != 0)
    {
        xmlFreeDoc(xml);
    }

    if ( ctx != 0)
    {
        xmlXPathFreeContext(ctx);
    }
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

vector<string> ObjectXML::operator[] (const char * xpath_expr)
{
    xmlXPathObjectPtr obj;
    vector<string>    content;

    obj = xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>(xpath_expr), ctx);

    if (obj == 0 || obj->nodesetval == 0)
    {
        return content;
    }

    xmlNodeSetPtr ns = obj->nodesetval;
    int           size = ns->nodeNr;
    xmlNodePtr    cur;
    xmlChar *     str_ptr;

    for(int i = 0; i < size; ++i)
    {
        cur = ns->nodeTab[i];

        if ( cur == 0 || cur->type != XML_ELEMENT_NODE )
        {
            continue;
        }

        str_ptr = xmlNodeGetContent(cur);

        if (str_ptr != 0)
        {
            string element_content = reinterpret_cast<char *>(str_ptr);

            content.push_back(element_content);

            xmlFree(str_ptr);
        }
    }

    xmlXPathFreeObject(obj);

    return content;
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int ObjectXML::xpath(string& value, const char * xpath_expr, const char * def)
{
    vector<string> values;
    int rc = 0;

    values = (*this)[xpath_expr];

    if ( values.empty() == true )
    {
        value = def;
        rc    = -1;
    }
    else
    {
        value = values[0];
    }

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int ObjectXML::xpath(int& value, const char * xpath_expr, const int& def)
{
    vector<string> values;
    int rc = 0;

    values = (*this)[xpath_expr];

    if (values.empty() == true)
    {
        value = def;
        rc = -1;
    }
    else
    {
        istringstream iss;

        iss.str(values[0]);

        iss >> dec >> value;

        if (iss.fail() == true)
        {
            value = def;
            rc    = -1;
        }
    }

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int ObjectXML::xpath(unsigned int& value, const char * xpath_expr,
                     const unsigned int& def)
{
    vector<string> values;
    int rc = 0;

    values = (*this)[xpath_expr];

    if (values.empty() == true)
    {
        value = def;
        rc = -1;
    }
    else
    {
        istringstream iss;

        iss.str(values[0]);

        iss >> dec >> value;

        if (iss.fail() == true)
        {
            value = def;
            rc    = -1;
        }
    }

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int ObjectXML::xpath(time_t& value, const char * xpath_expr, const time_t& def)
{
    int int_val;
    int int_def = static_cast<time_t>(def);
    int rc;

    rc = xpath(int_val, xpath_expr, int_def);
    value = static_cast<time_t>(int_val);

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int ObjectXML::xpath_value(string& value,const char *doc,const char *the_xpath)
{
    int rc = 0;

    try
    {
        ObjectXML      obj(doc);
        vector<string> values;

        values = obj[the_xpath];

        if (values.empty() == true)
        {
            rc = -1;
        }
        else
        {
            value = values[0];
        }
    }
    catch(runtime_error& re)
    {
        rc = -1;
    }

    return rc;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int ObjectXML::get_nodes (const char * xpath_expr, vector<xmlNodePtr>& content)
{
    xmlXPathObjectPtr obj;

    obj = xmlXPathEvalExpression(
        reinterpret_cast<const xmlChar *>(xpath_expr), ctx);

    if (obj == 0 || obj->nodesetval == 0)
    {
        return 0;
    }

    xmlNodeSetPtr ns = obj->nodesetval;
    int           size = ns->nodeNr;
    int           num_nodes = 0;
    xmlNodePtr    cur;

    for(int i = 0; i < size; ++i)
    {
        cur = xmlCopyNode(ns->nodeTab[i],1);

        if ( cur == 0 || cur->type != XML_ELEMENT_NODE )
        {
            xmlFreeNode(cur);
            continue;
        }

        content.push_back(cur);
        num_nodes++;
    }

    xmlXPathFreeObject(obj);

    return num_nodes;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int ObjectXML::update_from_str(const string &xml_doc)
{
    if (xml != 0)
    {
        xmlFreeDoc(xml);
    }

    if ( ctx != 0)
    {
        xmlXPathFreeContext(ctx);
    }

    try
    {
        xml_parse(xml_doc);
    }
    catch(runtime_error& re)
    {
        return -1;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int ObjectXML::update_from_node(const xmlNodePtr node)
{
    if (xml != 0)
    {
        xmlFreeDoc(xml);
    }

    if ( ctx != 0)
    {
        xmlXPathFreeContext(ctx);
    }

    xml = xmlNewDoc(reinterpret_cast<const xmlChar *>("1.0"));

    if (xml == 0)
    {
        return -1;
    }

    ctx = xmlXPathNewContext(xml);

    if (ctx == 0)
    {
        xmlFreeDoc(xml);
        return -1;
    }

    xmlNodePtr root_node = xmlDocCopyNode(node,xml,1);

    if (root_node == 0)
    {
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(xml);
        return -1;
    }

    xmlDocSetRootElement(xml, root_node);

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int ObjectXML::validate_xml(const string &xml_doc)
{
    xmlDocPtr tmp_xml = 0;
    tmp_xml = xmlParseMemory (xml_doc.c_str(),xml_doc.length());

    if (tmp_xml == 0)
    {
        return -1;
    }

    xmlFreeDoc(tmp_xml);

    return 0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void ObjectXML::xml_parse(const string &xml_doc)
{
    xml = xmlParseMemory (xml_doc.c_str(),xml_doc.length());

    if (xml == 0)
    {
        throw runtime_error("Error parsing XML Document");
    }

    ctx = xmlXPathNewContext(xml);

    if (ctx == 0)
    {
        xmlFreeDoc(xml);
        throw runtime_error("Unable to create new XPath context");
    }
}


/* ************************************************************************ */
/* Host :: Parse functions to compute rank and evaluate requirements        */
/* ************************************************************************ */

extern "C"
{
    typedef struct yy_buffer_state * YY_BUFFER_STATE;

    int expr_bool_parse(ObjectXML * oxml, bool& result, char ** errmsg);

    int expr_arith_parse(ObjectXML * oxml, int& result, char ** errmsg);

    int expr_lex_destroy();

    YY_BUFFER_STATE expr__scan_string(const char * str);

    void expr__delete_buffer(YY_BUFFER_STATE);
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

int ObjectXML::eval_bool(const string& expr, bool& result, char **errmsg)
{
    YY_BUFFER_STATE     str_buffer = 0;
    const char *        str;
    int                 rc;

    *errmsg = 0;

    str = expr.c_str();

    str_buffer = expr__scan_string(str);

    if (str_buffer == 0)
    {
        goto error_yy;
    }

    rc = expr_bool_parse(this,result,errmsg);

    expr__delete_buffer(str_buffer);

    expr_lex_destroy();

    return rc;

error_yy:

    *errmsg=strdup("Error setting scan buffer");

    return -1;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

int ObjectXML::eval_arith(const string& expr, int& result, char **errmsg)
{
    YY_BUFFER_STATE     str_buffer = 0;
    const char *        str;
    int                 rc;

    *errmsg = 0;

    str = expr.c_str();

    str_buffer = expr__scan_string(str);

    if (str_buffer == 0)
    {
        goto error_yy;
    }

    rc = expr_arith_parse(this,result,errmsg);

    expr__delete_buffer(str_buffer);

    expr_lex_destroy();

    return rc;

error_yy:

    *errmsg=strdup("Error setting scan buffer");

    return -1;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

