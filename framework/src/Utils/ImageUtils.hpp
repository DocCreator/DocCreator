#ifndef IMAGE_UTILS_HPP
#define IMAGE_UTILS_HPP

#include <QImage>
#include <QStringList>
#include <framework_global.h>

/**
   @return image @a img converted to gray.
   
   Returned image is a 32-bit image.
 */
extern FRAMEWORK_EXPORT QImage toGray(const QImage &img);

/** 
    @return filter to use to load/read images 

    Filter is "Image (*.bmp *.png *.jpg ... )"
 */
extern FRAMEWORK_EXPORT QString getReadImageFilter();

/** 
    @return filter to use to save/write images 

    Filter is "Image (*.bmp *.png *.jpg ... )"
 */
extern FRAMEWORK_EXPORT QString getWriteImageFilter();

/** 
    @return filter list to use to load/read images 

    Return a list (for example containing *.png *.jpg ...)
    suitable for nameFilters of QDir for example.
*/
extern FRAMEWORK_EXPORT QStringList getReadImageFilterList();

/** 
    @return filter list to use to save/write images 
*/
extern FRAMEWORK_EXPORT QStringList getWriteImageFilterList();


#endif /* ! IMAGE_UTILS_HPP */ 
