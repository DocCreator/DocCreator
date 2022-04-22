
# DocCreator FAQ 


## Image degradation by batch

In DocCreator, you can process images in batches from the GUI using the little black factory icon in the toolbar.
It also possible to process images from the command line (See Image degradation from the command line)


## Image degradation from the command line

DocCreator provides a command line tool, DocCreatorDegradator. It allows to apply the various degradation effects to a given images directory. You will need to compile DocCreator from sources (and thus previously install Qt, cmake and OpenCV). You will also have to configure cmake with the option -DBUILD_OTHER_PROGS=ON and do "make install".
These tool applies all the degradations. If you want to apply only a specific effect E1, you will have to set do_E1=true and do_E2=false for E2!=E1 in software/DocCreator/src/Degradator/main.cpp and compile again.

Work on a python binding is currently done in the pybind11 branch to be able to use DocCreator degradations directly from python.


## Non-latin characters & languages

DocCreator actually consists of two parts:

 - 1) create document images from scratch (using a specific background image, text and font)
 - 2) add degradation effects to the images of existing documents

The first part, document image generation, is currently not usable for non-latin, left-to-right top-to-bottom written, documents. Handling this is currently worked on on the pybind11 branch. 
The second part, adding degradation effects to the images of existing documents, can be used on any images, in particular document images with any language.


## Adding New Fonts

The fonts in DocCreator (that we call "old-fonts") are "bitmap" fonts. Each character is associated with one or more images of this character. The character is not drawn from curves as in a TrueType font. Having different images for a given character allows us to reproduce the diversity of characters found in old documents.

There is a tool directly in DocCreator to manually extract such a new "old-font" from a document image. It allows to have several images for each character.
There is also an independent tool, DocCreatorMakeFont, provided with DocCreator, that allows to produce an "old-font" for use in DocCreator from an (installed) TrueType font. However, this "old-font" will only have one image for each character. You will need to configure DocCreator with -DBUILD_OTHER_PROGS=ON to build such a tool.


## Adding 3D Meshes 

For the 3D meshes, you can use any 3D mesh with parameterization, that is, with texture coordinates, in .obj format.
.brs is just a more compressed (custom) way to store an .obj file.
So if you can create a 3D mesh, with a mesh editor (for example with Blender or Meshlab) or programmatically (for example with the VCG library used by Meshlab), compute the texture coordinates to map the document image to the mesh, and save as .obj, you should be able to use it in DocCreator.

DocCreator provides a way to compute the texture coordinates, but it will only work if your 3D mesh is relatively planar.
14 out of the 17 meshes available in DocCreator (Vesale*.brs and livre*.brs) were obtained with a 3D scanner from real books, then parameterized this way.

If you configure/build DocCreator with -DBUILD_OTHER_PROGS_3D=ON, you will have a tool (tools/convertBRSOBJ) to convert a file from .obj format to .brs format.

Besides, in the "pybind11" branch, DocCreator also offers a 2D "warp" effect, called "elastic deformation". It allows to have a quite similar effect without the need of a 3D Mesh. It also allows to pass bounding boxes (of words for example) and get back the bounding boxes transformed by the effect. See https://github.com/DocCreator/DocCreator/blob/pybind11/framework/src/Degradations/ElasticDeformation.hpp 

