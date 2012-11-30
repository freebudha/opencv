#include <iostream>
#include "cvconfig.h"

#ifndef HAVE_OPENGL
int main()
{
    std::cerr << "Library was built without OpenGL support" << std::endl;
    return -1;
}
#else

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN 1
    #define NOMINMAX 1
    #include <windows.h>
#endif

#if defined(__APPLE__)
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include "opencv2/core/core.hpp"
#include "opencv2/core/opengl_interop.hpp"
#include "opencv2/core/gpumat.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

const int win_width = 800;
const int win_height = 640;

struct DrawData
{
    GlArrays arr;
    GlTexture2D tex;
    GlBuffer indices;
};

void CV_CDECL draw(void* userdata);

void CV_CDECL draw(void* userdata)
{
    static double angle = 0.0;

    DrawData* data = static_cast<DrawData*>(userdata);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)win_width / win_height, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);
    glRotated(angle, 0, 1, 0);

    glEnable(GL_TEXTURE_2D);
    data->tex.bind();

    glDisable(GL_CULL_FACE);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    render(data->arr, data->indices, RenderMode::TRIANGLES);

    angle += 0.3;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " image" << endl;
        return -1;
    }

    Mat img = imread(argv[1]);
    if (img.empty())
    {
        cerr << "Can't open image " << argv[1] << endl;
        return -1;
    }

    namedWindow("OpenGL", WINDOW_OPENGL);
    resizeWindow("OpenGL", win_width, win_height);

    Mat_<Vec2f> vertex(1, 4);
    vertex << Vec2f(-1, 1), Vec2f(-1, -1), Vec2f(1, -1), Vec2f(1, 1);

    Mat_<Vec2f> texCoords(1, 4);
    texCoords << Vec2f(0, 0), Vec2f(0, 1), Vec2f(1, 1), Vec2f(1, 0);

    Mat_<int> indices(1, 6);
    indices << 0, 1, 2, 2, 3, 0;

    DrawData data;

    data.arr.setVertexArray(vertex);
    data.arr.setTexCoordArray(texCoords);
    data.arr.setAutoRelease(false);

    data.indices.copyFrom(indices);
    data.indices.setAutoRelease(false);

    data.tex.copyFrom(img);
    data.tex.setAutoRelease(false);

    setOpenGlDrawCallback("OpenGL", draw, &data);

    for (;;)
    {
        updateWindow("OpenGL");
        int key = waitKey(10);
        if ((key & 0xff) == 27)
            break;
    }

    setOpenGlDrawCallback("OpenGL", 0, 0);
    destroyAllWindows();

    return 0;
}

#endif
