"""Tests of the python bindings of the degradation library.

The bindings had no test at all: every degradation is reached from python
through the ndarray converter, which has to decide, for an arbitrary numpy
array, whether it can be seen as an image and with which type. The cases
that matter are the ones a user hits by accident -- a float or a 16 bit
array, two channels, no pixel at all, more than three dimensions, a
non-contiguous slice -- and each of them has to be refused with an
exception rather than reinterpreted.

The seeded degradations are also checked end to end: the seed is the only
way a generated corpus can be reproduced, and it crosses the binding.
"""

import numpy as np
import pytest

import DocCreatorPy as dc


ROWS, COLS = 60, 80

SEED_A = 42
SEED_B = 43


def make_image(channels=None, dtype=np.uint8):
    """An image with two dark blocks on a light background."""
    shape = (ROWS, COLS) if channels is None else (ROWS, COLS, channels)
    img = np.full(shape, 210, dtype=dtype)
    img[10:30, 10:25] = 40
    img[10:30, 40:55] = 40
    return img


def make_pattern():
    """A CV_8UC1 hole/stain pattern: a white disc on black."""
    side = 30
    pattern = np.zeros((side, side), dtype=np.uint8)
    y, x = np.ogrid[:side, :side]
    pattern[(y - side // 2) ** 2 + (x - side // 2) ** 2 <= (side // 3) ** 2] = 255
    return pattern


#every shape the bindings document as accepted
GOOD_SHAPES = [None, 1, 3, 4]

#(description, array) pairs that must all be refused
def bad_images():
    return [
        ("uint16", make_image(dtype=np.uint16)),
        ("int8", make_image(dtype=np.int8)),
        ("float32", make_image(dtype=np.float32)),
        ("float64", make_image(dtype=np.float64)),
        ("bool", np.zeros((ROWS, COLS), dtype=bool)),
        ("two channels", make_image(channels=2)),
        ("five channels", make_image(channels=5)),
        ("empty", np.zeros((0, 0), dtype=np.uint8)),
        ("one dimension", np.zeros((ROWS,), dtype=np.uint8)),
        ("no dimension", np.uint8(7)),
        ("four dimensions", np.zeros((2, ROWS, COLS, 2), dtype=np.uint8)),
        ("not an array", [[1, 2], [3, 4]]),
        ("none", None),
    ]


def call_with(img, name):
    """Call the degradation @a name on @a img with otherwise valid arguments."""
    calls = {
        "characterDegradation": lambda i: dc.characterDegradation(i, 5, .33, .33, SEED_A),
        "addGaussianNoise": lambda i: dc.addGaussianNoise(i, 0., 20., dc.AddNoiseType.AS_IS, SEED_A),
        "addSpeckleNoise": lambda i: dc.addSpeckleNoise(i, 0., .5, dc.AddNoiseType.AS_IS, SEED_A),
        "addSaltAndPepperNoise": lambda i: dc.addSaltAndPepperNoise(i, .15, .5, SEED_A),
        "blur": lambda i: dc.blur(i, dc.BlurMethod.GAUSSIAN, 3),
        "elasticTransform": lambda i: dc.elasticTransform(i, 20., .08, dc.ElasticBorderReplication.BLACK, dc.ElasticInterpolation.BILINEAR, SEED_A),
        "elasticTransform2": lambda i: dc.elasticTransform2(i, 20., .08, 9., dc.ElasticBorderReplication.BLACK, dc.ElasticInterpolation.BILINEAR, SEED_A),
        "rotateFillColor": lambda i: dc.rotateFillColor(i, 5., (0., 0., 0., 255.)),
        "rotateFillBorder": lambda i: dc.rotateFillBorder(i, 5., dc.RotationBorderReplication.WRAP),
        "rotateFillInpaint1": lambda i: dc.rotateFillInpaint1(i, 5., .05),
        "shadowBinding": lambda i: dc.shadowBinding(i, dc.ShadowBorder.TOP, 10, .5, 45.),
        "shadowBinding2": lambda i: dc.shadowBinding2(i, .2, dc.ShadowBorder.TOP, .5, 45.),
        "addHoleAtRandom": lambda i: dc.addHoleAtRandom(i, make_pattern(), 0, dc.HoleType.CENTER, 0., dc.HoleSide.BORDER_TOP, (0., 0., 0., 255.), None, 0, .1, SEED_A),
    }
    return calls[name](img)


#the degradations above, which all take one image and give one back
ONE_IMAGE_DEGRADATIONS = [
    "characterDegradation",
    "addGaussianNoise",
    "addSpeckleNoise",
    "addSaltAndPepperNoise",
    "blur",
    "elasticTransform",
    "elasticTransform2",
    "rotateFillColor",
    "rotateFillBorder",
    "rotateFillInpaint1",
    "shadowBinding",
    "shadowBinding2",
    "addHoleAtRandom",
]

#those of them that take a seed
SEEDED_DEGRADATIONS = [
    "characterDegradation",
    "addGaussianNoise",
    "addSpeckleNoise",
    "addSaltAndPepperNoise",
    "elasticTransform",
    "elasticTransform2",
    "addHoleAtRandom",
]


@pytest.mark.parametrize("name", ONE_IMAGE_DEGRADATIONS)
@pytest.mark.parametrize("channels", GOOD_SHAPES)
def test_accepts_every_documented_shape(name, channels):
    img = make_image(channels)
    before = img.copy()

    out = call_with(img, name)

    assert isinstance(out, np.ndarray)
    assert out.dtype == np.uint8
    #a single channel image comes back without its channel axis: there is
    #no such thing as a one channel cv::Mat with three dimensions
    expected = (ROWS, COLS) if channels in (None, 1) else img.shape
    assert out.shape == expected
    #the input is never modified in place
    assert np.array_equal(img, before)


@pytest.mark.parametrize("name", ONE_IMAGE_DEGRADATIONS)
@pytest.mark.parametrize("description,img", bad_images(),
                         ids=[d for d, _ in bad_images()])
def test_refuses_what_is_not_an_image(name, description, img):
    with pytest.raises((ValueError, TypeError)):
        call_with(img, name)


@pytest.mark.parametrize("name", ONE_IMAGE_DEGRADATIONS)
def test_accepts_a_non_contiguous_view(name):
    #a slice of a larger array has a row stride that is not its width
    whole = make_image(3)
    view = whole[:, ::2]
    assert not view.flags["C_CONTIGUOUS"]

    out = call_with(view, name)
    assert out.shape == view.shape
    assert out.dtype == np.uint8


@pytest.mark.parametrize("name", SEEDED_DEGRADATIONS)
def test_the_seed_crosses_the_binding(name):
    img = make_image(3)

    first = call_with(img, name)
    again = call_with(img, name)
    assert np.array_equal(first, again)

    #and a different seed gives something else
    def with_seed_b(i):
        return {
            "characterDegradation": lambda: dc.characterDegradation(i, 5, .33, .33, SEED_B),
            "addGaussianNoise": lambda: dc.addGaussianNoise(i, 0., 20., dc.AddNoiseType.AS_IS, SEED_B),
            "addSpeckleNoise": lambda: dc.addSpeckleNoise(i, 0., .5, dc.AddNoiseType.AS_IS, SEED_B),
            "addSaltAndPepperNoise": lambda: dc.addSaltAndPepperNoise(i, .15, .5, SEED_B),
            "elasticTransform": lambda: dc.elasticTransform(i, 20., .08, dc.ElasticBorderReplication.BLACK, dc.ElasticInterpolation.BILINEAR, SEED_B),
            "elasticTransform2": lambda: dc.elasticTransform2(i, 20., .08, 9., dc.ElasticBorderReplication.BLACK, dc.ElasticInterpolation.BILINEAR, SEED_B),
            "addHoleAtRandom": lambda: dc.addHoleAtRandom(i, make_pattern(), 0, dc.HoleType.CENTER, 0., dc.HoleSide.BORDER_TOP, (0., 0., 0., 255.), None, 0, .1, SEED_B),
        }[name]()

    other = with_seed_b(img)
    assert not np.array_equal(first, other)


def test_no_seed_is_accepted():
    #the seed is optional, and None means a non reproducible one
    img = make_image(3)
    assert dc.characterDegradation(img, 5, .33, .33).shape == img.shape
    assert dc.characterDegradation(img, 5, .33, .33, None).shape == img.shape


class TestPositionTuples:
    """The tuple arguments go through their own converters."""

    def test_bleed_through_takes_a_position(self):
        recto = make_image(3)
        verso = make_image(3)[:, ::-1].copy()
        out = dc.bleedThrough(recto, verso, 5, (0, 0), 1)
        assert out.shape == recto.shape

    @pytest.mark.parametrize("pos", [(), (1,), (1, 2, 3), "xy", 3, None,
                                     (1.5, "a")])
    def test_bleed_through_refuses_a_bad_position(self, pos):
        recto = make_image(3)
        verso = make_image(3)
        with pytest.raises((ValueError, TypeError)):
            dc.bleedThrough(recto, verso, 5, pos, 1)

    def test_add_hole_takes_a_position_and_a_colour(self):
        img = make_image(3)
        out = dc.addHole(img, make_pattern(), (10, 10), 0,
                         dc.HoleType.CENTER, dc.HoleSide.BORDER_TOP,
                         (0., 0., 0., 255.))
        assert out.shape == img.shape

    @pytest.mark.parametrize("color", [(1.,), (1., 2.), (1., 2., 3.),
                                       (1., 2., 3., 4.)])
    def test_add_hole_takes_a_colour_of_one_to_four_components(self, color):
        #a cv::Scalar is one to four doubles, the missing ones being zero
        img = make_image(3)
        out = dc.addHole(img, make_pattern(), (10, 10), 0,
                         dc.HoleType.CENTER, dc.HoleSide.BORDER_TOP, color)
        assert out.shape == img.shape

    @pytest.mark.parametrize("color", [(), (1., 2., 3., 4., 5.),
                                       "rgba", None, (1., "a", 3., 4.)])
    def test_add_hole_refuses_a_bad_colour(self, color):
        img = make_image(3)
        with pytest.raises((ValueError, TypeError)):
            dc.addHole(img, make_pattern(), (10, 10), 0,
                       dc.HoleType.CENTER, dc.HoleSide.BORDER_TOP, color)


class TestTwoImageDegradations:
    """The degradations that take a second image check it too."""

    def test_bleed_through_refuses_a_verso_of_another_type(self):
        recto = make_image(3)
        verso = make_image(1)
        with pytest.raises((ValueError, TypeError)):
            dc.bleedThrough(recto, verso, 5, (0, 0), 1)

    def test_rotate_fill_image_refuses_a_background_of_another_type(self):
        img = make_image(3)
        background = make_image(dtype=np.uint16)
        with pytest.raises((ValueError, TypeError)):
            dc.rotateFillImage(img, 5., background)

    def test_add_hole_refuses_a_pattern_that_is_not_an_image(self):
        img = make_image(3)
        with pytest.raises((ValueError, TypeError)):
            dc.addHole(img, np.zeros((0, 0), dtype=np.uint8), (10, 10), 0,
                       dc.HoleType.CENTER, dc.HoleSide.BORDER_TOP,
                       (0., 0., 0., 255.))


class TestOutOfRangeValues:
    """Out-of-range parameters are refused, not clamped silently."""

    @pytest.mark.parametrize("ratios", [(-.1, .5), (.5, -.1), (1.1, 0.),
                                        (.6, .6)])
    def test_character_degradation_ratios(self, ratios):
        img = make_image(3)
        with pytest.raises((ValueError, TypeError)):
            dc.characterDegradation(img, 5, ratios[0], ratios[1], SEED_A)

    @pytest.mark.parametrize("amount", [-.1, 1.1])
    def test_salt_and_pepper_amount(self, amount):
        img = make_image(3)
        with pytest.raises((ValueError, TypeError)):
            dc.addSaltAndPepperNoise(img, amount, .5, SEED_A)
