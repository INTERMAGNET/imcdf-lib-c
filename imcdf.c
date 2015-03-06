/*****************************************************************************
 * imcdf.c - routines to read and write a CDF data file in the INTERMAGNET
 *           format
 *
 * THE IMCDF ROUTINES SHOULD NOT HAVE DEPENDENCIES ON OTHER LIBRARY ROUTINES -
 * IT MUST BE POSSIBLE TO DISTRIBUTE THE IMCDF SOURCE CODE
 *
 * CDF - http://cdf.gsfc.nasa.gov/
 * INTERMAGNET - www.intermagnet.org
 *
 * To read an ImagCDF file:
 *        Call imcdf_open2 ()
 *        Call imcdf_read_global_attrs () to read the global attributes
 *                and to discover what information is in the file
 *              Use the IMCDFGlobalAttr.elements_recorded field to discover which
 *                geomagnetic elements are recorded in the file. For temparture variables
 *                start at Temperature1 and increment the number calling the read routine
 *                to check whether the variable exists in the file
 *        Call imcdf_read_variable multiple () times, once for each field
 *                element or temperature that you wish to retrive from the file
 *              Call imcdf_read_time_stamps() to read the time stamps for the variables
 *        Call imcdf_close2 ()
 *         Call imcdf_free_global_attrs () and imcdf_free_variable () and
 *                imcdf_free_time_stamps () to free memory the was allocated
 *                when reading attributes, variables and time stamps
 *
 * To write an ImagCDF file:
 *        Call imcdf_open2 ()
 *        Call imcdf_write_global_attrs () to write the global attributes
 *        Call imcdf_write_variable multiple () times, once for each field element
 *                or temperature that you wish to put in the file
 *              Call imcdf_write_time_stamps () to write the time stamps
 *        Call imcdf_close2 ()
 *
 * Simon Flower, 20/12/2012
 * Updates to version 1.1 of ImagCDF. Simon Flower, 19/02/2015 
 *****************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
 
#include "imcdf.h"

/* private forward declarations */
static int is_blank (char *s);
static char *create_var_name (enum IMCDFVariableType var_type, char *elem_rec);

/** ------------------------------------------------------------------------
 *  ---- Open and close (using character based error return as for all -----
 *  ---- other functions in this source code file)                     -----
 *  ------------------------------------------------------------------------*/

/*****************************************************************************
 * imcdf_open2
 *
 * Description: open a CDF for reading or writing
 *
 * Input parameters: filename - the CDF file to open
 *                   open_type - how to open the file
 *                   compress_type - how to compress the file (only used
 *                                   on file that is being created)
 * Output parameters: handle - handle to the open CDF file
 * Returns: null for success, an error message if there was a fault, in which
 *          case handle will not contain a valid value
 *
 *****************************************************************************/
char *imcdf_open2 (char *filename, enum IMCDFOpenType open_type, 
                   enum IMCDFCompressionType compress_type, int *cdf_handle)
{

    *cdf_handle = imcdf_open (filename, open_type, compress_type);
    if (*cdf_handle < 0)
        return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    return 0;
    
}

/*****************************************************************************
 * imcdf_close2
 *
 * Description: close a CDF - you MUST call this after writing to the CDF 
 *              otherwise it will be corrupt
 *
 * Input parameters: cdf_handle - the CDF to close
 * Output parameters: none
 * Returns: null for success, an error message if there was a fault
 *
 *****************************************************************************/
char *imcdf_close2 (int cdf_handle)
 
{
    if (imcdf_close (cdf_handle))
        return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    return 0;

}
 
/** ------------------------------------------------------------------------
 *  ----------------------- Reading from CDF files -------------------------
 *  ------------------------------------------------------------------------*/
 
/*****************************************************************************
 * imcdf_read_global_attrs
 *
 * Description: Read the global attributes from an ImagCDF file, also read
 *              'discovery' information that allows the variables in the file
 *              to be read
 *
 * Input parameters: cdf_handle - handle to the CDF file
 * Output parameters: global_attrs - the attributes
 * Returns: null for success, an error message if there was a fault
 *
 *****************************************************************************/
char *imcdf_read_global_attrs (int cdf_handle, struct IMCDFGlobalAttr *global_attrs)

{

    int vt_count, found;
    char var_name [30], *pl_str, *sl_str, *str;

    /* get the global attributes */
    if (imcdf_get_global_attribute_string (cdf_handle, "FormatDescription",    0, &(global_attrs->format_description))) return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "FormatVersion",        0, &(global_attrs->format_version)))     return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "Title",                0, &(global_attrs->title)))              return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "IagaCode",             0, &(global_attrs->iaga_code)))          return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "ElementsRecorded",     0, &(global_attrs->elements_recorded)))  return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "PublicationLevel",     0, &pl_str))                             return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_tt2000 (cdf_handle, "PublicationDate",      0, &(global_attrs->pub_date)))           return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "ObservatoryName",      0, &(global_attrs->observatory_name)))   return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_double (cdf_handle, "Latitude",             0, &(global_attrs->latitude)))           return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_double (cdf_handle, "Longitude",            0, &(global_attrs->longitude)))          return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_double (cdf_handle, "Elevation",            0, &(global_attrs->elevation)))          return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "Institution",          0, &(global_attrs->institution)))        return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "VectorSensOrient",     0, &(global_attrs->vector_sens_orient))) global_attrs->vector_sens_orient = 0;
    if (imcdf_get_global_attribute_string (cdf_handle, "StandardLevel",        0, &sl_str))                             return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "StandardName",         0, &(global_attrs->standard_name)))      global_attrs->standard_name = 0;;
    if (imcdf_get_global_attribute_string (cdf_handle, "StandardVersion",      0, &(global_attrs->standard_version)))   global_attrs->standard_version = 0;
    if (imcdf_get_global_attribute_string (cdf_handle, "PartialStandDesc",     0, &(global_attrs->partial_stand_desc))) global_attrs->partial_stand_desc = 0;;
    if (imcdf_get_global_attribute_string (cdf_handle, "Source",               0, &(global_attrs->source)))             return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_global_attribute_string (cdf_handle, "TermsOfUse",           0, &(global_attrs->terms_of_use)))       global_attrs->terms_of_use = 0;
    if (imcdf_get_global_attribute_string (cdf_handle, "UniqueIdentifier",     0, &(global_attrs->unique_identifier)))  global_attrs->unique_identifier = 0;
    global_attrs->pub_level = imcdf_parse_pub_level_string (pl_str);
    global_attrs->standard_level = imcdf_parse_standard_level_string (sl_str);
    free (pl_str);
    free (sl_str);
    global_attrs->parent_identifiers = 0;
    global_attrs->n_parent_identifiers = 0;
    for (found=1; found; )
    {
          if (! imcdf_get_global_attribute_string (cdf_handle, "ParentIdentifiers", global_attrs->n_parent_identifiers, &(str)))
        {
            global_attrs->parent_identifiers = realloc (global_attrs->parent_identifiers, sizeof (char *) * (global_attrs->n_parent_identifiers +1));
            if (! global_attrs->parent_identifiers)
                return imcdf_status_code_tostring (BAD_MALLOC);
            global_attrs->parent_identifiers [global_attrs->n_parent_identifiers ++] = str;
        }
        else
            found = 0;
    }
    global_attrs->reference_links = 0;
    global_attrs->n_reference_links = 0;
    for (found=1; found; )
    {
          if (! imcdf_get_global_attribute_string (cdf_handle, "ReferenceLinks", global_attrs->n_reference_links, &(str)))
        {
            global_attrs->parent_identifiers = realloc (global_attrs->reference_links, sizeof (char *) * (global_attrs->n_reference_links +1));
            if (! global_attrs->reference_links)
                return imcdf_status_code_tostring (BAD_MALLOC);
            global_attrs->reference_links [global_attrs->n_reference_links ++] = str;
        }
        else
            found = 0;
    }

    /* check metadata */
    if (strcasecmp (global_attrs->title,              "Geomagnetic time series data")) return "Error: Title of data incorrect";
    if (strcasecmp (global_attrs->format_description, "INTERMAGNET CDF Format"))       return "Error: Description of data incorrect";
    if (strcasecmp (global_attrs->format_version,     "1.1"))                          return "Error: Format incorrect";
    
    return 0;
}

/*****************************************************************************
 * imcdf_read_variable
 *
 * Description: read a variable and its metadata from an ImagCDF file
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                   var_type - the variable type
 *                   elem_rec - H,D,Z... for geomagnetic data, 1,2,3.. for temperature data
 * Output parameters: var - the variable and it's metadata
 * Returns: null for success, an error message if there was a fault
 *
 *****************************************************************************/
char *imcdf_read_variable (int cdf_handle, enum IMCDFVariableType var_type, 
                           char *elem_rec, struct IMCDFVariable *variable)

{
    char var_name [30], *msg, *ptr;
    static char errmsg [100];
    
    /* create the variable name */
    ptr = create_var_name (var_type, elem_rec);
    if (! ptr) return "Error: Invalid variable type";
    strcpy (var_name, ptr);
    
    /* read the variable metadata */
    variable->var_type = var_type;
    strcpy (variable->elem_rec, elem_rec);
    if (imcdf_get_variable_attribute_string (cdf_handle, "FIELDNAM",  var_name, &(variable->field_nam)))  return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_variable_attribute_string (cdf_handle, "UNITS",     var_name, &(variable->units)))      return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_variable_attribute_double (cdf_handle, "FILLVAL",   var_name, &(variable->fill_val)))   return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_variable_attribute_double (cdf_handle, "VALIDMIN",  var_name, &(variable->valid_min)))  return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_variable_attribute_double (cdf_handle, "VALIDMAX",  var_name, &(variable->valid_max)))  return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_get_variable_attribute_string (cdf_handle, "DEPEND_0",  var_name, &(variable->depend_0)))   return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    
    /* read the data */
    variable->data = imcdf_get_var_data (cdf_handle, var_name, &(variable->data_len));
    if (! variable->data) 
    {
      if (imcdf_get_last_status_code () == CDF_OK) sprintf (errmsg, "Error reading variable data %s", var_name);
      else (strcpy (errmsg, imcdf_status_code_tostring (imcdf_get_last_status_code ())));
      return errmsg;
    }
        
    return 0;

}


/*****************************************************************************
 * imcdf_read_time_stamps
 *
 * Description: read a time stamp variable from an ImagCDF file
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                   var_name - name of the variable that holds the time stamps
 * Output parameters: ts - the time stamp data
 * Returns: null for success, an error message if there was a fault
 *
 *****************************************************************************/
char *imcdf_read_time_stamps (int cdf_handle, char *var_name, struct IMCDFVariableTS *ts)

{

    static char errmsg [100];

    ts->var_name = var_name;
    ts->time_stamps = imcdf_get_var_time_stamps (cdf_handle, var_name, &(ts->data_len));
    if (! ts->time_stamps)
    {
        if (imcdf_get_last_status_code () == CDF_OK) sprintf (errmsg, "Error reading time stamps %s", var_name);
        else (strcpy (errmsg, imcdf_status_code_tostring (imcdf_get_last_status_code ())));
        return errmsg;
    }
        
    return 0;

}

    
/*****************************************************************************
 * imcdf_free_global_attrs
 *
 * Description: Free the memory allocated after a successful call to 
 *                imcdf_read_global_attrs ()
 *
 * Input parameters: global_attrs - the global attributes structure passed to
 *                                    imcdf_read_global_attrs ()
 * Output parameters: 
 * Returns: 
 *
 *****************************************************************************/
void imcdf_free_global_attrs (struct IMCDFGlobalAttr *global_attrs)
{
    int count;
    
    free (global_attrs->format_description);
    free (global_attrs->format_version);
    free (global_attrs->title);
    free (global_attrs->iaga_code);
    free (global_attrs->elements_recorded);
    free (global_attrs->observatory_name);
    free (global_attrs->institution);
    if (global_attrs->vector_sens_orient) free (global_attrs->vector_sens_orient);
    if (global_attrs->standard_name) free (global_attrs->standard_name);
    if (global_attrs->standard_version) free (global_attrs->standard_version);
    if (global_attrs->partial_stand_desc) free (global_attrs->partial_stand_desc);
    free (global_attrs->source);    
    if (global_attrs->terms_of_use) free (global_attrs->terms_of_use);
    if (global_attrs->unique_identifier) free (global_attrs->unique_identifier);
    for (count=0; count<global_attrs->n_parent_identifiers; count++)
        free (global_attrs->parent_identifiers + count);
    if (global_attrs->parent_identifiers) free (global_attrs->parent_identifiers);
    for (count=0; count<global_attrs->n_reference_links; count++)
        free (global_attrs->reference_links + count);
    if (global_attrs->reference_links) free (global_attrs->reference_links);
}

/*****************************************************************************
 * imcdf_free_variable
 *
 * Description: Free the memory allocated after a successful call to 
 *                imcdf_read_variable ()
 *
 * Input parameters: variable - the variable structure passed to
 *                                imcdf_read_variable ()
 * Output parameters: 
 * Returns: 
 *
 *****************************************************************************/
void imcdf_free_variable (struct IMCDFVariable *variable)

{
    free (variable->field_nam);
    free (variable->units);
    free (variable->depend_0);
    free (variable->data);
}

/*****************************************************************************
 * imcdf_free_time_stamps
 *
 * Description: Free the memory allocated after a successful call to 
 *                imcdf_read_time_stamps ()
 *
 * Input parameters: ts - the time stamp structure passed to
 *                                    imcdf_read_time_stamps ()
 * Output parameters: 
 * Returns: 
 *
 *****************************************************************************/
void imcdf_free_time_stamps (struct IMCDFVariableTS *ts)

{
    free (ts->time_stamps);
}

/** ------------------------------------------------------------------------
 *  ------------------------ Writing to CDF files --------------------------
 *  ------------------------------------------------------------------------*/
    
/*****************************************************************************
 * imcdf_write_global_attrs
 *
 * Description: Write the global attributes to an ImagCDF file
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                   global_attrs - the attributes
 * Output parameters: some fields in global_attrs will be over written if they
 *                    are blank: title; format_description; format_version;
 *                    terms_of_use
 * Returns: null for success, an error message if there was a fault
 *
 *****************************************************************************/
char *imcdf_write_global_attrs (int cdf_handle, struct IMCDFGlobalAttr *global_attrs)

{
    int count;
    char date_string [30];

    /* default values */
    if (is_blank (global_attrs->title))              global_attrs->title = "Geomagnetic time series data";
    if (is_blank (global_attrs->format_description)) global_attrs->format_description = "INTERMAGNET CDF Format";
    if (is_blank (global_attrs->format_version))     global_attrs->format_version = "1.1";
    if (is_blank (global_attrs->terms_of_use))       global_attrs->terms_of_use = getINTERMAGNETTermsOfUse();
         
    /* write metadata */
    if (imcdf_add_global_attr_string (cdf_handle, "FormatDescription", 0,                                  global_attrs->format_description))    return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "FormatVersion", 0,                                      global_attrs->format_version))        return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "Title", 0,                                              global_attrs->title))                 return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "IagaCode", 0,                                           global_attrs->iaga_code))             return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "ElementsRecorded", 0,                                   global_attrs->elements_recorded))     return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "PublicationLevel", 0,    imcdf_pub_level_code_tostring (global_attrs->pub_level)))            return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_tt2000 (cdf_handle, "PublicationDate", 0,                                    global_attrs->pub_date))              return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "ObservatoryName", 0,                                    global_attrs->observatory_name))      return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_double (cdf_handle, "Latitude", 0,                                           global_attrs->latitude))              return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_double (cdf_handle, "Longitude", 0,                                          global_attrs->longitude))             return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_double (cdf_handle, "Elevation", 0,                                          global_attrs->elevation))             return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "Institution", 0,                                        global_attrs->institution))           return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "VectorSensOrient", 0,                                   global_attrs->vector_sens_orient))    return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "StandardLevel", 0,  imcdf_standard_level_code_tostring (global_attrs->standard_level)))       return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "StandardName", 0,                                       global_attrs->standard_name))         return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "StandardVersion", 0,                                    global_attrs->standard_version))      return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "PartialStandDesc", 0,                                   global_attrs->partial_stand_desc))    return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "Source", 0,                                             global_attrs->source))                return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "TermsOfUse", 0,                                         global_attrs->terms_of_use))          return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_global_attr_string (cdf_handle, "UniqueIdentifier", 0,                                   global_attrs->unique_identifier))     return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    for (count=0; count<global_attrs->n_parent_identifiers; count++)
    {
        if (imcdf_add_global_attr_string (cdf_handle, "ParentIdentifiers", count, global_attrs->parent_identifiers [count])) return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    }
    for (count=0; count<global_attrs->n_reference_links; count++)
    {
        if (imcdf_add_global_attr_string (cdf_handle, "ReferenceLinks", count, global_attrs->reference_links [count])) return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    }

    return 0;
}

/*****************************************************************************
 * imcdf_write_variable
 *
 * Description: write a variable and its metadata to an ImagCDF file
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                   variable - the variable to write
 * Output parameters: none
 * Returns: null for success, an error message if there was a fault
 *
 *****************************************************************************/
char *imcdf_write_variable (int cdf_handle, struct IMCDFVariable *variable)

{

    char var_name [30], depend_0 [50], *ptr, lablaxis [20];
    
    /* create the variable name */
    ptr = create_var_name (variable->var_type, variable->elem_rec);
    if (! ptr) return "Error: Invalid variable type";
    strcpy (var_name, ptr);
    
    /* write the data */
    if (imcdf_create_data_array (cdf_handle, var_name, variable->data, variable->data_len)) return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    
    /* write the metadata - ignore the DEPEND_0 value in the 'variable' structure and construct a value from the data */
    if (imcdf_is_vector_gm_data (variable->var_type, variable->elem_rec))
        strcpy (depend_0, VECTOR_TIME_STAMPS_VAR_NAME);
    else if (imcdf_is_scalar_gm_data (variable->var_type, variable->elem_rec))
        strcpy (depend_0, SCALAR_TIME_STAMPS_VAR_NAME);
    else
    {
        if (variable->var_type != IMCDF_VARTYPE_TEMPERATURE)
            return "Missing or invalid element code";
        sprintf (depend_0, TEMPERATURE_TIME_STAMPS_VAR_NAME_BASE, variable->elem_rec);
    }

    if (variable->var_type == IMCDF_VARTYPE_TEMPERATURE)
        sprintf (lablaxis, "Temperature %s", variable->elem_rec);
    else
        strcpy (lablaxis, variable->elem_rec);
    
    if (imcdf_add_variable_attr_string (cdf_handle, "FIELDNAM",      var_name, variable->field_nam)) return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_variable_attr_string (cdf_handle, "UNITS",         var_name, variable->units))     return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_variable_attr_double (cdf_handle, "FILLVAL",       var_name, variable->fill_val))  return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_variable_attr_double (cdf_handle, "VALIDMIN",      var_name, variable->valid_min)) return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_variable_attr_double (cdf_handle, "VALIDMAX",      var_name, variable->valid_max)) return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_variable_attr_string (cdf_handle, "DEPEND_0",      var_name, depend_0))            return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_variable_attr_string (cdf_handle, "DISPLAY_TYPE",  var_name, "time_series"))       return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    if (imcdf_add_variable_attr_string (cdf_handle, "LABLAXIS",      var_name, lablaxis))            return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    
    return 0;
}

/*****************************************************************************
 * imcdf_write_time_stamps
 *
 * Description: write a set of time stamps to an ImagCDF file
 *
 * Input parameters: cdf_handle - handle to the CDF file
 *                   ts - the time stamps to write
 * Output parameters: none
 * Returns: null for success, an error message if there was a fault
 *
 *****************************************************************************/
char *imcdf_write_time_stamps (int cdf_handle, struct IMCDFVariableTS *ts)

{
    
    /* write the data */
    if (imcdf_create_time_stamp_array (cdf_handle, ts->var_name, ts->time_stamps, ts->data_len))
        return imcdf_status_code_tostring (imcdf_get_last_status_code ());
    return 0;
    
}

/** ------------------------------------------------------------------------
 *  ---------------------------- Useful utilities --------------------------
 *  ------------------------------------------------------------------------*/

char *getINTERMAGNETTermsOfUse ()
{
    return "CONDITIONS OF USE FOR DATA PROVIDED THROUGH INTERMAGNET:\n"
           "The data made available through INTERMAGNET are provided for\n"
           "your use and are not for commercial use or sale or distribution\n"
           "to third parties without the written permission of the institute\n"
           "(http://www.intermagnet.org/Institutes_e.html) operating\n"
           "the observatory. Publications making use of the data\n"
           "should include an acknowledgment statement of the form given below.\n"
           "A citation reference should be sent to the INTERMAGNET Secretary\n"
           "(secretary@intermagnet.org) for inclusion in a publications list\n"
           "on the INTERMAGNET website.\n"
           "\n"
           "     ACKNOWLEDGEMENT OF DATA FROM OBSERVATORIES\n"
           "     PARTICIPATING IN INTERMAGNET\n"
           "We offer two acknowledgement templates. The first is for cases\n"
           "where data from many observatories have been used and it is not\n"
           "practical to list them all, or each of their operating institutes.\n"
           "The second is for cases where research results have been produced\n"
           "using a smaller set of observatories.\n"
           "\n"
           "     Suggested Acknowledgement Text (template 1)\n"
           "The results presented in this paper rely on data collected\n"
           "at magnetic observatories. We thank the national institutes that\n"
           "support them and INTERMAGNET for promoting high standards of\n"
           "magnetic observatory practice (www.intermagnet.org).\n"
           "\n"
           "     Suggested Acknowledgement Text (template 2)\n"
           "The results presented in this paper rely on the data\n"
           "collected at <observatory name>. We thank <institute name>,\n"
           "for supporting its operation and INTERMAGNET for promoting high\n"
           "standards of magnetic observatory practice (www.intermagnet.org).\n";
}
    
/****************************************************************************
 * imcdf_is_vector_gm_data
 *
 * Description: decide whether an element code represents a vector element
 *
 * Input parameters: var_type - the type of variable
 *                   elem_rec - the element code
 * Output parameters:
 * Returns: true if this element represents vector geomagnetic data, 
 *          false otherwise
 *
 ****************************************************************************/
int imcdf_is_vector_gm_data (enum IMCDFVariableType var_type, char *elem_rec)

{
    if (var_type != IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT)
        return 0;
    switch (toupper (*elem_rec))
    {
        case 'X':
        case 'Y':
        case 'Z':
        case 'H':
        case 'D':
        case 'E':
        case 'V':
        case 'I':
        case 'F':
            return 1;
    }
    return 0;
}
    
/****************************************************************************
 * imcdf_is_scalar_gm_data
 *
 * Description: decide whether an element code represents a vector element
 *
 * Input parameters: var_type - the type of variable
 *                   elem_rec - the element code
 * Output parameters:
 * Returns: true if this element represents vector geomagnetic data, 
 *          false otherwise
 *
 ****************************************************************************/
int imcdf_is_scalar_gm_data (enum IMCDFVariableType var_type, char *elem_rec)

{
    if (var_type != IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT)
        return 0;
    switch (toupper (*elem_rec))
    {
        case 'S':
        case 'G':
            return 1;
    }
    return 0;
}

/****************************************************************************
 * imcdf_make_filename
 *
 * Description: generate an IMAG CDF filename
 *
 * Input parameters: prefix - the prefix for the name (including any directory)
 *                   station_code - the IAGA station code
 *                   start_date - the start date for the data (in CDF TT2000 format)
 *                   pub_leve - the publication level of the data
 *                   sample_period - the period between samples, in seconds
 *                   force_lower_case - set true to force the filename to lower case
 * Output parameters: filename - the filename
 * Returns: the filename OR null if there is an error
 *
 ****************************************************************************/
char *imcdf_make_filename (char *prefix, char *station_code, long long start_date,
                           enum IMCDFPubLevel pub_level,
                           double sample_period,
                           int force_lower_case, char *filename)
{
    int year, month, day, hour, min, sec;
    char file_base [20], *ptr;
    double dummy;

    /* convert the date to a format that can be printed */
    imcdf_tt2000_to_date_time (start_date, &year, &month, &day, &hour, &min, &sec);

    /* set up the parts of the filename that depend on the sample period */
    if (sample_period <= 1.0)               /* second data */
        sprintf (file_base, "%04d%02d%02d_%02d%02d%02d", year, month, day, hour, min, sec);
    else if (sample_period <= 60.0)         /* minute data */
        sprintf (file_base, "%04d%02d%02d_%02d%02d", year, month, day, hour, min);
    else if (sample_period <= 3600.0)       /* hour data */
        sprintf (file_base, "%04d%02d%02d_%02d", year, month, day, hour);
    else if (sample_period <= 86400.0)      /* day data */
        sprintf (file_base, "%04d%02d%02d", year, month, day);
    else if (sample_period <= 2678400)      /* month data */
        sprintf (file_base, "%04d%02d", year, month);
    else                                    /* year data */
        sprintf (file_base, "%04d", year);

    if (! prefix) prefix = "";
    sprintf (filename, "%s%s_%s_%s.cdf", prefix, station_code, file_base, 
         imcdf_pub_level_code_tostring (pub_level));

    if (force_lower_case)
    {
        /* there isn't a strlwr function in the standard library and we don't
         * want to make this code dependent on any other libraries apart
         * from CDF, so convert to lower case char by char... */
        for (ptr = filename + strlen (prefix); *ptr; ptr++)
            *ptr = tolower (*ptr);
    }
    return filename;
}


/** ------------------------------------------------------------------------
 *  ---------------------------- Private code ------------------------------
 *  ------------------------------------------------------------------------*/
    
static int is_blank (char *s)
{
    if (! s) return 1;
    if (! *s) return 1;
    return 0;
}
    
static char *create_var_name (enum IMCDFVariableType var_type, char *elem_rec)
{
    static char var_name [30];
    
    switch (var_type)
    {
    case IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT:
        sprintf (var_name, "GeomagneticField%s", elem_rec);
        break;
    case IMCDF_VARTYPE_TEMPERATURE:
        sprintf (var_name, "Temperature%s", elem_rec);
        break;
    default:
        return 0;
    }
    return var_name;
}    


    



    
    
