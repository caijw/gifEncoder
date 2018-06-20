{
  "targets": [
    {
      "target_name": "gifNodeAddOn",
      "sources": [
        "./src/stb_image.h",
        "./src/ImageBuffer.cpp",
        "./src/GifFrameEncoder.cpp",
        "./src/LzwEncoder.cpp",
        "./src/NeuQuant.cpp",
        "./src/Logger.cpp",
        "./src/GifEncoder.cpp",
        "./src/main.cpp"
      ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")"
      ]

    }
  ]
}