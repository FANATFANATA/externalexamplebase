#ifndef WATERMARK_H
#define WATERMARK_H

namespace watermark
{

    struct WatermarkConfig
    {
        bool showVersion = true;
        bool showType = true;
        bool showFps = true;
    };

    WatermarkConfig &GetWatermarkConfig();
    void DrawWatermark(bool &menu_visible);

}

#endif
