#include "btm_renderer.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION  // optional. disable exception handling.
#include "tiny_gltf.h"

namespace btm
{

BaseRenderer::BaseRenderer(Ref<btm::Window> window)
{
    mWindowHandle = window->handle();
    mViewportSize = window->size();

    BTM_ASSERT_X(mWindowHandle, "Invalid window handle");
    BTM_ASSERT_X(mViewportSize.x > 0 && mViewportSize.y > 0, "Invalid viewport size");
}

// Load a glTF scene from a file path

auto getVec2   = [](float const *d, int i) { return glm::vec2 { d[i + 0], d[i + 1] }; };
auto getVec3   = [](float const *d, int i) { return glm::vec3 { d[i + 0], d[i + 1], d[i + 2] }; };
auto getVec4   = [](float const *d, int i) { return glm::vec4 { d[i + 0], d[i + 1], d[i + 2], d[i + 3] }; };
auto getFloats = [](float const *d, int i) { return glm::vec4 { d[i + 0], d[i + 1], d[i + 2], d[i + 3] }; };

std::vector<Mesh> parseGltf(std::string const &filepath)
{
    //-------------------------------------
    // READ FILE FROM DISK
    //-------------------------------------

    std::ifstream   file { filepath, std::ios::binary };
    auto            fileBegin = std::istreambuf_iterator<char>(file);
    auto            fileEnd   = std::istreambuf_iterator<char>();
    std::vector<u8> raw { fileBegin, fileEnd };

    bool const isBinary = raw[0] == 'g' and raw[1] == 'l' and raw[2] == 'T' and raw[3] == 'F';

    BTM_INFOF("parseGltf => {} : {}", filepath, isBinary);

    if (raw.empty())
    {
        BTM_ERRF("File '{}' empty or invalid", filepath);
        BTM_ASSERT(0);
        return {};
    }

    //-------------------------------------
    // PARSE FILE WITH TINYGLTF
    //-------------------------------------

    tinygltf::Model    model;
    tinygltf::TinyGLTF ctx;
    std::string        err, warn;

    bool const ok = !isBinary ? ctx.LoadASCIIFromFile(&model, &err, &warn, filepath)
                              : ctx.LoadBinaryFromMemory(&model, &err, &warn, raw.data(), raw.size());

    if (!err.empty())
        BTM_ERRF("Loading GLTF '{}' : {}", filepath, err);
    if (!warn.empty())
        BTM_WARNF("Loading GLTF '{}' : {}", filepath, warn);
    if (!ok)
        BTM_WARNF("Loading GLTF '{}' : Undefined error", filepath);
    if (!err.empty() or !warn.empty() or !ok)
        return {};

    //-------------------------------------
    // POPULATE MESHES ARRAY
    //-------------------------------------

    std::vector<Mesh> meshes;

    for (auto const &mesh : model.meshes)
    {
        Mesh outMesh;
        outMesh.name = mesh.name;

        // ... Populate mesh
        //-------------------------------------
        for (const auto &primitive : mesh.primitives)
        {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                continue;

            static std::array const sAttributes = {
                std::make_tuple(Mesh::Pos, 3, "POSITION"),    std::make_tuple(Mesh::UV0, 2, "TEXCOORD_Ø"),
                std::make_tuple(Mesh::Normal, 3, "NORMAL"),   std::make_tuple(Mesh::Tangent, 4, "TANGENT"),
                std::make_tuple(Mesh::Tangent, 4, "TANGENT"),
            };

            /* INDICES
            //=============================================
              // Get the accessor index for the indices of the primitive
              const int indicesAccessorIndex = primitive.indices;
              // Get the accessor for the indices
              const tinygltf::Accessor &indicesAccessor = model.accessors[indicesAccessorIndex];
              // Get the buffer view index for the indices
              const int indicesBufferViewIndex = indicesAccessor.bufferView;
              // Get the buffer view for the indices
              const tinygltf::BufferView &indicesBufferView = model.bufferViews[indicesBufferViewIndex];
              // Get the buffer index for the indices
              const int indicesBufferIndex = indicesBufferView.buffer;
              // Get the buffer for the indices
              const tinygltf::Buffer &indicesBuffer = model.buffers[indicesBufferIndex];
              // Get the data pointer for the indices
              const void *indicesData = &indicesBuffer.data[indicesBufferView.byteOffset];
              // Cast the data pointer to the desired data type (for example, uint16_t)
              const uint16_t *indicesArray = static_cast<const uint16_t *>(indicesData);
              // The number of elements in the indices array is determined by the count property of the accessor
              const int indicesCount = indicesAccessor.count;
              // Do something with the indices array, for example, print it
              for (int i = 0; i < indicesCount; i++) {
                std::cout << indicesArray[i] << " ";
              }
            */

            for (auto const &[type, components, name] : sAttributes)
            {
                // ... Gather indices
                //-------------------------------------
                int accessorIdx   = -1;
                int bufferViewIdx = -1;

                //@dani : Segment the process in helper functions, it will help to simplify Indicies read also.
                for (auto const &attribute : primitive.attributes)
                {
                    if (attribute.first == name)
                    {
                        accessorIdx   = attribute.second;
                        bufferViewIdx = model.accessors[accessorIdx].bufferView;
                        break;
                    }
                }
                if (accessorIdx < 0 or bufferViewIdx < 0)
                {
                    // BTM_WARNF("Parsing {} : accessor or buffer not found", name);
                    continue;
                }

                BTM_INFOF("iiiiiiiiindex => {}/{}", accessorIdx, bufferViewIdx);

                // ... Gather data
                //-------------------------------------
                tinygltf::Accessor const   &accessor    = model.accessors[accessorIdx];
                size_t const                numVertices = accessor.count / components;
                tinygltf::BufferView const &view        = model.bufferViews[bufferViewIdx];
                tinygltf::Buffer const     &buffer      = model.buffers[view.buffer];
                int32_t const               offset      = view.byteOffset + accessor.byteOffset;
                float const                *data        = reinterpret_cast<float const *>(&buffer.data[offset]);

                // BTM_INFOF("Parsing {} : {} vertices (size {})", name, numVertices, components);

                // ... Store data in atrributes
                //-------------------------------------
                outMesh.vertices.resize(numVertices);

                for (size_t i = 0; i < numVertices; ++i)
                {
                    size_t curr = components * i;
                    auto  &v    = outMesh.vertices[i];

                    switch (type)
                    {
                        case Mesh::Pos: v.pos = getVec3(data, curr); break;
                        case Mesh::UV0: v.uv0 = getVec2(data, curr); break;
                        case Mesh::Normal: v.normal = getVec3(data, curr); break;
                        case Mesh::Tangent: v.tangent = getVec4(data, curr); break;
                        default: break;
                    }
                }
            }
        }

        meshes.push_back(outMesh);
    }

    //-------------------------------------

    BTM_INFOF("END parseGltf => {} : {}", filepath, meshes);

    // if (meshes.size() > 0)
    //     for (auto const &v : meshes.at(0).normals)
    //         BTM_INFOF("aaaaaaaaaaaaaaaaaaaaa {}", v);

    return meshes;
}

}  // namespace btm
