/*****************************************************************************
 * imcdf_utils.c - utilities for the C version of ImagCDF - the INTERMAGNET 
 *                 CDF format
 *
 * THE IMCDF ROUTINES SHOULD NOT HAVE DEPENDENCIES ON OTHER LIBRARY ROUTINES -
 * IT MUST BE POSSIBLE TO DISTRIBUTE THE IMCDF SOURCE CODE
 *
 * Simon Flower, 18/12/2012
 * Updates to version 1.1 of ImagCDF. Simon Flower, 19/02/2015  
 *****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
 
#include "imcdf.h"
 
 /*****************************************************************************
  * imcdf_parse_pub_level_string
  *
  * Description: parse a string that contains an ImagCDF publication level description
  *
  * Input parameters: string - the string to parse
  * Output parameters:
  * Returns: the coded publication level code
  *
  *****************************************************************************/
enum IMCDFPubLevel imcdf_parse_pub_level_string (char *string)

{

    if (! strcasecmp (string, "1"))
        return IMCDF_PUBLEVEL_1;
    if (! strcasecmp (string, "2"))
        return IMCDF_PUBLEVEL_2;
    if (! strcasecmp (string, "3"))
        return IMCDF_PUBLEVEL_3;
    if (! strcasecmp (string, "4"))
        return IMCDF_PUBLEVEL_4;

    return IMCDF_PUBLEVEL_1;
}

 /*****************************************************************************
  * imcdf_pub_level_code_tostring
  *
  * Description: convert a publication level code to a string for storage in a CDF file
  *              or for use in a filename
  *
  * Input parameters: code - the code to display
  * Output parameters:
  * Returns: a string interpretation of the code
  *
  *****************************************************************************/
char *imcdf_pub_level_code_tostring (enum IMCDFPubLevel code)

{

    switch (code)
    {
    case IMCDF_PUBLEVEL_1: return "1";
    case IMCDF_PUBLEVEL_2: return "2";
    case IMCDF_PUBLEVEL_3: return "3";
    case IMCDF_PUBLEVEL_4: return "4";
    }
    
    return "1";
}

 /*****************************************************************************
  * imcdf_parse_standard_level_string
  *
  * Description: parse a string that contains an ImagCDF standard conformance
  *                 level description
  *
  * Input parameters: string - the string to parse
  * Output parameters:
  * Returns: the coded standards conformance code
  *
  *****************************************************************************/
enum IMCDFStandardLevel imcdf_parse_standard_level_string (char *string)

{

    if (! strcasecmp (string, "full"))
        return IMCDF_STANDLEVEL_FULL;
    if (! strcasecmp (string, "partial"))
        return IMCDF_STANDLEVEL_PARTIAL;
    if (! strcasecmp (string, "none"))
        return IMCDF_STANDLEVEL_NONE;

    return IMCDF_STANDLEVEL_NONE;
}

 /*****************************************************************************
  * imcdf_standard_level_code_tostring
  *
  * Description: convert a standard level code to a string for storage in a CDF file
  *
  * Input parameters: code - the code to display
  * Output parameters:
  * Returns: a string interpretation of the code
  *
  *****************************************************************************/
char *imcdf_standard_level_code_tostring (enum IMCDFStandardLevel code)

{

    switch (code)
    {
    case IMCDF_STANDLEVEL_FULL: return "Full";
    case IMCDF_STANDLEVEL_PARTIAL: return "Partial";
    case IMCDF_STANDLEVEL_NONE: return "None";
    }
    
    return "None";
}

 /*****************************************************************************
  * imcdf_parse_var_type_string
  *
  * Description: parse a string that contains an ImagCDF variable type
  *                 description
  *
  * Input parameters: string - the string to parse
  * Output parameters:
  * Returns: the coded variable type code
  *
  *****************************************************************************/
enum IMCDFVariableType imcdf_parse_var_type_string (char *string)

{

    if (! strcasecmp (string, "GeomagneticFieldElement"))
        return IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT;
    else if (! strcasecmp (string, "Temperature"))
        return IMCDF_VARTYPE_TEMPERATURE;

    return IMCDF_VARTYPE_ERROR;
}

 /*****************************************************************************
  * imcdf_var_type_code_tostring
  *
  * Description: convert a variable type code to a string
  *
  * Input parameters: code - the code to display
  * Output parameters:
  * Returns: a string interpretation of the code
  *
  *****************************************************************************/
char *imcdf_var_type_code_tostring (enum IMCDFVariableType code)

{

    switch (code)
    {
    case IMCDF_VARTYPE_GEOMAGNETIC_FIELD_ELEMENT: return "GeomagneticFieldElement";
    case IMCDF_VARTYPE_TEMPERATURE: return "Temperature";
    }
    
    return "Error";
}

 /*****************************************************************************
  * imcdf_get_var_name
  *
  * Description: create the name of a variable that will be used in a CDF file
  *
  * Input parameters: var_type - the variable type for this variable 
  *                   element_code - the element code for this variable 
  *                                  H, D, Z, etc. for geomagnetic elements
  *                                  1, 2, 3, etc. for temperatures
  * Output parameters: var_name - the variable name
  * Returns: the variable name 
  *
  *****************************************************************************/
char *imcdf_get_var_name (enum IMCDFVariableType code, char element_code, char *var_name)

{
    sprintf (var_name, "%s%c", imcdf_var_type_code_tostring (code), element_code);
    return var_name;

}
    
/*******************************************************************
 * imcdf_dt_to_publevel
 *
 * Description: convert an IMF or IAGA-2002 data type to IMCDF
 *              publication level
 *
 * Input parameters: dt - the IMF or IAGA data type string (full
 *                        or abbreviated, e.g. 'R' or 'reported'
 * Output parameters: none
 * Returns the publication state or baseline type
 *
 * Comments:
 *
 ******************************************************************/
enum IMCDFPubLevel imcdf_dt_to_pub_level (char *dt)
{
    switch (toupper (*dt))
    {
    case 'V':
    case 'R': return IMCDF_PUBLEVEL_1;
    case 'P':
    case 'A': return IMCDF_PUBLEVEL_2;
    case 'Q': return IMCDF_PUBLEVEL_3;
    case 'D': return IMCDF_PUBLEVEL_4;
    }
    return IMCDF_PUBLEVEL_1;
}

/*******************************************************************
 * imcdf_print_global_attrs
 * imcdf_print_variable
 *
 * Description: dump the global attributes data to stdout
 *              dump a variable, its metadata and time stamps to stdout
 *
 * Input parameters: global_attrs - the CDF file global attributes
 *                   variable - a variable from the CDF file
 *                   time_stamps - time stamps correspoding to the variable
 * Output paramters: none
 * Returns: none
 *******************************************************************/
void imcdf_print_global_attrs (struct IMCDFGlobalAttr *global_attrs)
{
    int count;

    printf ("ImagCDF Global Attributes:\n");
    printf ("    FormatDescription: %s\n", global_attrs->format_description);
    printf ("    FormatVersion: %s\n", global_attrs->format_version);
    printf ("    Title: %s\n", global_attrs->title);
    printf ("    IagaCode: %s\n", global_attrs->iaga_code);
    printf ("    ElementsRecorded: %s\n", global_attrs->elements_recorded);
    printf ("    PublicationLevel: %s\n", imcdf_pub_level_code_tostring (global_attrs->pub_level));
    printf ("    PublicationDate: %s\n", imcdf_tt2000_tostring (global_attrs->pub_date));
    printf ("    ObservatoryName: %s\n", global_attrs->observatory_name);
    printf ("    Latitude: %f\n", global_attrs->latitude);
    printf ("    Longitude: %f\n", global_attrs->longitude);
    printf ("    Elevation: %f\n", global_attrs->elevation);
    printf ("    Institution: %s\n", global_attrs->institution);
    printf ("    VectorSensOrient: %s\n", global_attrs->vector_sens_orient ? global_attrs->vector_sens_orient : "");
    printf ("    StandardLevel: %s\n", imcdf_standard_level_code_tostring (global_attrs->standard_level));
    printf ("    StandardName: %s\n", global_attrs->standard_name ? global_attrs->standard_name : "");
    printf ("    StandardVersion: %s\n", global_attrs->standard_version ? global_attrs->standard_version : "");
    printf ("    PartialStandDesc: %s\n", global_attrs->partial_stand_desc ? global_attrs->partial_stand_desc : "");
    printf ("    Source: %s\n", global_attrs->source);
    printf ("    TermsOfUse: %50.50s\n", global_attrs->terms_of_use ? global_attrs->terms_of_use : "");
    printf ("    UniqueIdentifier: %s\n", global_attrs->unique_identifier ? global_attrs->unique_identifier : "");
    for (count=0; count<global_attrs->n_parent_identifiers; count++)
    {
        if (count == 0)
            printf ("    ParentIdentifiers: %s\n", *(global_attrs->parent_identifiers + count));
        else
            printf ("                       %s\n", *(global_attrs->parent_identifiers + count));
    }
    for (count=0; count<global_attrs->n_reference_links; count++)
    {
        if (count == 0)
            printf ("    ReferenceLinks: %s\n", *(global_attrs->reference_links + count));
        else
            printf ("                    %s\n", *(global_attrs->reference_links + count));
    }
}

void imcdf_print_variable (struct IMCDFVariable *variable, struct IMCDFVariableTS *time_stamps)
{
    int count;

    printf ("ImagCDF Variable %s %s\n",
            imcdf_var_type_code_tostring (variable->var_type), variable->elem_rec);
    printf ("    FIELDNAM: %s\n", variable->field_nam);
    printf ("    UNITS: %s\n", variable->units);
    printf ("    FILLVAL: %f\n", variable->fill_val);
    printf ("    VALIDMIN: %f\n", variable->valid_min);
    printf ("    VALIDMAX: %f\n", variable->valid_max);
    printf ("    Depend_0: %s\n", variable->depend_0);
    printf ("    Data length: %d\n", variable->data_len);
    printf ("    Time stamps from: %s\n", time_stamps->var_name);

    for (count=0; count<variable->data_len; count++)
    {
        if (count < time_stamps->data_len)
            printf ("      %s ", imcdf_tt2000_tostring (*(time_stamps->time_stamps + count)));
        else
            printf ("      Missing time stamp ");

        printf ("%.3f\n", *(variable->data + count));
    }
}



