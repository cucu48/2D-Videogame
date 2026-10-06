#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

#include <array>
#include <vector>
#include <map>
#include <memory>

using namespace std;

typedef array<unsigned char, 4> pixel;
typedef array<array<pixel, 16>, 16> tile;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./level_converter input.png output.png\n";
        return 1;
    }

    int width, height, originalChannels;

    // Request 4 channels per pixel: red, green, blue, alpha.
    unique_ptr<unsigned char, decltype(&stbi_image_free)> pixels(stbi_load(
        argv[1], &width, &height, &originalChannels, 4
    ), stbi_image_free);

    if (!pixels) {
        cerr << "Could not load image: "
                  << stbi_failure_reason() << '\n';
        return 1;
    }

    if (width % 16 != 0 || height % 16 != 0) {
        std::cerr << "Image dimensions must be multiples of 16\n";
        return 1;
    }

    //Custom logic
    map<tile, int> tileMap;
    vector< vector<int> > level(height / 16, vector<int>(width / 16));
    for (int y = 0; y < height; y += 16) {
        for (int x = 0; x < width; x += 16) {
            tile currentTile;
            for (int ty = 0; ty < 16; ++ty) {
                for (int tx = 0; tx < 16; ++tx) {
                    size_t offset = (size_t(y + ty) * width + (x + tx)) * 4;
                    for (int channel = 0; channel < 4; ++channel) {
                        currentTile[ty][tx][channel] = pixels.get()[offset + channel];
                    }
                }
            }

            if(tileMap.find(currentTile) == tileMap.end()) {
                int id = static_cast<int>(tileMap.size());
                tileMap.emplace(currentTile, id);
            }

            level[y / 16][x / 16] = tileMap.at(currentTile);
        }
    }

    // Use power-of-two dimensions, with enough slots for every unique tile.
    size_t widthTileMap = 1;
    while (widthTileMap * widthTileMap < tileMap.size()) {
        widthTileMap *= 2;
    }
    size_t heightTileMap = (widthTileMap/2 * widthTileMap < tileMap.size())? widthTileMap : widthTileMap/2;
    
    const int atlasWidth = static_cast<int>(widthTileMap * 16);
    const int atlasHeight = static_cast<int>(heightTileMap * 16);
    vector<unsigned char> atlas(size_t(atlasWidth) * atlasHeight * 4, 0);

    for(map<tile, int>::iterator it = tileMap.begin(); it != tileMap.end(); it++) {
        int key = it->second;
        for(int y = 0; y < 16; y+=1) {
            for(int x = 0; x < 16; x+=1) {
                size_t atlasX = (key % widthTileMap) * 16 + x;
                size_t atlasY = (key / widthTileMap) * 16 + y;
                size_t offset = (atlasY * atlasWidth + atlasX) * 4;
                for (int channel = 0; channel < 4; ++channel) {
                    atlas[offset + channel] = it->first[y][x][channel];
                }
            }
        }
    }

    int saved = stbi_write_png(
        argv[2], atlasWidth, atlasHeight, 4, atlas.data(), atlasWidth * 4
    );

    if (!saved) {
        std::cerr << "Could not save output image\n";
        return 1;
    }

    cout << "TILEMAP\n" << width/16 << ' ' << height /16 << "\n16 16\n" << argv[2] << '\n' << widthTileMap << ' ' << heightTileMap << '\n';

    for (int y = 0; y < height/16; y+=1) {
        for (int x = 0; x < width/16; x+=1) {
            cout << level[y][x];
            if(x != width/16-1) cout << ',';
        }    
        if(y != height/16-1) cout << '\n';
    }

    return 0;
}
