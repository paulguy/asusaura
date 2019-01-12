#ifndef COMPYGLOW_H
#define COMPYGLOW_H

#include "arglist.h"

/*
 * Usage Summary
 *
 * First a list of arguments should be prepared, then the library needs to be
 * opened with the arguments passed in.  The count of devices will be returned.
 * Number of devices may be queried at any time, and any errors or other events
 * after function calls may cause the number to change.  Names may be queried.
 * An array which should be used for filling in color values can be prepared
 * which should then be initialized and passed in to update.  Once you're done
 * with this structure it may be freed.  When done the library should be closed.
 */

/*
 * Structure containing color value.
 *
 * r
 *   Red magnitude.
 *
 * g
 *   Green magnitude.
 *
 * b
 *   Blue magnitude.
 */
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} CompyGlow_Color;

/*
 * Open the library.
 *
 * args
 *   List of arguments, see arglist.h on building an ArgList.
 *
 * return
 *   Number of devices successfully opened.  Should only be called once per 
 *   execution or per closing.
 */
unsigned int CompyGlow_open(ArgList *args);

/*
 * Close the library.
 *
 * return
 *   Number of devices successfully closed.  Shouldn't be called until opened
 *   again, even if less devices were closed than opened.
 */
unsigned int CompyGlow_close();

/*
 * Get the count of open devices, should be the same as returned when opening
 * but modules may add or remove devices at any time so the value may change.
 *
 * return
 *   Number of open devices.
 */
unsigned int CompyGlow_getDevCount();

/*
 * Get the name of an open device.
 *
 * idx
 *   Index of device.
 *
 * return
 *   Name of an device or NULL if out of range.
 */
char *CompyGlow_getDevName(unsigned int idx);

/*
 * Get an array of structures which should have r, g and b values assigned to
 * them and passed to update.  This array may be freed when no longer needed.
 *
 * idx
 *   Index of device.
 *
 * c
 *   Pointer to array of structures to use with update.
 *
 * return
 *   Number of LEDs for this device.
 */
unsigned int CompyGlow_getColorReq(unsigned int idx, CompyGlow_Color **c);

/*
 * Update LEDs to values provided in the array of Color structures.
 *
 * idx
 *   Index of device.
 *
 * c
 *   Array of Color structures which must have at least as many values as the
 *   device requires, acquired by getColorReq.
 *
 * return
 *   0 on success, negative on failure.
 */
int CompyGlow_update(unsigned int idx, CompyGlow_Color *c);

#endif
