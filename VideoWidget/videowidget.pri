HEADERS += \
    $$PWD/render/factory.h \
    $$PWD/render/nv12render.h \
    $$PWD/render/videorender.h \
    $$PWD/render/yuvrender.h \
    $$PWD/renderthread.h \
    $$PWD/videodecode/decodetaskmanagerimpl.h \
    $$PWD/videodecode/decodtask.h \
    $$PWD/videodecode/ffmpegcpudecode.h \
    $$PWD/videodecode/ffmpegcudadecode.h \
    $$PWD/videodecode/ffmpegqsvdecode.h \
    $$PWD/videodecode/nvidia_decoer_api.h \
    $$PWD/videodecode/nvidiadecode.h \
    $$PWD/videowidget.h

SOURCES += \
    $$PWD/render/nv12render.cpp \
    $$PWD/render/yuvrender.cpp \
    $$PWD/renderthread.cpp \
    $$PWD/videodecode/decodetaskmanagerimpl.cpp \
    $$PWD/videodecode/decodtask.cpp \
    $$PWD/videodecode/ffmpegcpudecode.cpp \
    $$PWD/videodecode/ffmpegcudadecode.cpp \
    $$PWD/videodecode/ffmpegqsvdecode.cpp \
    $$PWD/videodecode/nvidiadecode.cpp \
    $$PWD/videowidget.cpp
