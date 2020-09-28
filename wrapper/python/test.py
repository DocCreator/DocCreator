import DocCreator
import numpy as np

img1 = (np.random.rand(180, 100, 1) * 255).astype(np.uint8)
img2 = (np.random.rand(180, 100, 1) * 255).astype(np.uint8)


img3 = DocCreator.grayscaleCharsDegradation(img1, level=5, I=33.0, O=33.0, D=34.0)
img3 = DocCreator.bleedThrough(img3, img2, nbIter=30, x=20, y=20, nbThreads=-1)
img3 = DocCreator.blur(img3, method=0, intensity=5)
img3 = DocCreator.shadowBinding(img3, border=3, distance=img1.shape[0]//3, intensity=0.9, angle=80)
img3 = DocCreator.addGaussianNoise(img3)
img3 = DocCreator.rotateFillImageRepeats(img3, angle=11.2, imgBackground=img3, repeats=3)
img3 = DocCreator.elasticDeformation(img3)
