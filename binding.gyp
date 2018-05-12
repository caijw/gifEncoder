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
        "./src/GifFrameEncoder.cpp"

      ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        ["OS==\"linux\"", {
          "include_dirs": [
            "/usr/local/node-v9.11.1/include/node/"
          ]
        }]
      ]

    }
  ]
}