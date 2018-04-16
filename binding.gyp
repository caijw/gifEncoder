{
  "targets": [
    {
      "target_name": "gifNodeAddOn",
      "sources": [
        "./src/main.cpp",
        "./src/stb_image.h",
        "./src/GifEncoder.cpp",
        "./src/LzwEncoder.cpp",
        "./src/NeuQuant.cpp",
        "./src/Logger.cpp",
        "./src/ThreadPool.h"

      ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}