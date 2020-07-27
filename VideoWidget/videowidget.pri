{\rtf1}

HEADERS += \
    $$PWD/render/nv12render.h \
    $$PWD/render/rendermanager.h \
    $$PWD/render/videorender.h \
    $$PWD/render/yuvrender.h \
    $$PWD/videodecode/decodetaskmanagerimpl.h \
    $$PWD/videodecode/decodtask.h \
    $$PWD/videodecode/ffmpegcpudecode.h \
    $$PWD/videodecode/ffmpegcudadecode.h \
    $$PWD/videodecode/ffmpegqsvdecode.h \
    $$PWD/videowidget.h

SOURCES += \
    $$PWD/render/nv12render.cpp \
    $$PWD/render/rendermanager.cpp \
    $$PWD/render/yuvrender.cpp \
    $$PWD/videodecode/decodetaskmanagerimpl.cpp \
    $$PWD/videodecode/decodtask.cpp \
    $$PWD/videodecode/ffmpegcpudecode.cpp \
    $$PWD/videodecode/ffmpegcudadecode.cpp \
    $$PWD/videodecode/ffmpegqsvdecode.cpp \
    $$PWD/videowidget.cpp
