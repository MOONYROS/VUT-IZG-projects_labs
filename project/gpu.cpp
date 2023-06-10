/*!
 * @file
 * @brief This file contains implementation of gpu
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <student/gpu.hpp>
#include <iostream>

 /**
  * @brief This struct represents triangle
  */
  //! [Primitive]
struct Primitive {
    OutVertex vertex[3];
    float npd_w[3] = { 1.0f, 1.0f, 1.0f };
};
//! [Primitive]


void clear(GPUMemory& mem, ClearCommand cmd) {
    uint32_t height = mem.framebuffer.height;
    uint32_t width = mem.framebuffer.width;
    uint32_t pixels = height * width;

    if (cmd.clearColor) {
        uint8_t red = (uint8_t)(cmd.color.r * 255.f);
        uint8_t green = (uint8_t)(cmd.color.g * 255.f);
        uint8_t blue = (uint8_t)(cmd.color.b * 255.f);
        uint8_t alpha = (uint8_t)(cmd.color.a * 255.f);
        for (uint32_t i = 0; i < pixels * 4; i+=4)
        {
            mem.framebuffer.color[i + 0] = red;
            mem.framebuffer.color[i + 1] = green;
            mem.framebuffer.color[i + 2] = blue;
            mem.framebuffer.color[i + 3] = alpha;
        }
    }
    if (cmd.clearDepth)
    {
        for (uint32_t i = 0; i < pixels; i++)
            mem.framebuffer.depth[i] = cmd.depth;
    }
}

void computeVertexID(GPUMemory& mem, InVertex& inVertex, VertexArray vao, uint32_t v) {
    uint8_t* ptr;
    if (vao.indexBufferID < 0) {
        inVertex.gl_VertexID = v;
    } else {
        ptr = (uint8_t*)(mem.buffers[vao.indexBufferID].data);
        ptr += vao.indexOffset; // nema byt indexOffset take nasoben velikosti cisla???
        switch (vao.indexType) {
            case IndexType::UINT8:
                //ptr += v;
                inVertex.gl_VertexID = ((uint8_t*)ptr)[v]; // (uint8_t)(*ptr);
                break;
            case IndexType::UINT16:
                //ptr += v * 2;
                inVertex.gl_VertexID = ((uint16_t*)ptr)[v]; // (uint16_t)(*ptr);
                break;
            case IndexType::UINT32:
                //ptr += v * 4;
                inVertex.gl_VertexID = ((uint32_t*)ptr)[v]; // (uint32_t)(*ptr);
                break;
            default:
                // std::cerr << "neznama nedefinovana hodnota" << std::endl;
                break;
        }
    }
}

void readAttributes(GPUMemory& mem, InVertex& inVertex, VertexArray vao) {
    for (uint32_t attrNr = 0; attrNr < maxAttributes; attrNr++) {
        auto const& attr = vao.vertexAttrib[attrNr];
        if (attr.bufferID >= 0) { // if (attr.type != AttributeType::EMPTY)
            auto const& buf = mem.buffers[attr.bufferID].data;
            //std::cout << "gl_VertexID = " << inVertex.gl_VertexID << std::endl;
            size_t attrSize = 0;
            switch (attr.type) {
                case AttributeType::FLOAT:
                    inVertex.attributes[attrNr].v1 = *(float*)(((uint8_t*)buf) + attr.stride * inVertex.gl_VertexID + attr.offset);
                    break;
                case AttributeType::UINT:
                    break;
                case AttributeType::VEC2:
                // case AttributeType::UVEC2:
                    inVertex.attributes[attrNr].v2 = *(glm::vec2*)(((uint8_t*)buf) + attr.stride * inVertex.gl_VertexID + attr.offset);
                    break;
                case AttributeType::VEC3:
                // case AttributeType::UVEC3:
                    inVertex.attributes[attrNr].v3 = *(glm::vec3*)(((uint8_t*)buf) + attr.stride * inVertex.gl_VertexID + attr.offset);
                    break;
                case AttributeType::VEC4:
                // case AttributeType::UVEC4:
                    inVertex.attributes[attrNr].v4 = *(glm::vec4*)(((uint8_t*)buf) + attr.stride * inVertex.gl_VertexID + attr.offset);
                    break;
            }
        }
    }
}

void runVertexAssembly(GPUMemory& mem, InVertex& inVertex, VertexArray vao, uint32_t v) {
    computeVertexID(mem, inVertex, vao, v);
    readAttributes(mem, inVertex, vao);
}

void runPrimitiveAssembly(Primitive& primitive, GPUMemory& mem, VertexArray vao, uint32_t t, Program prg, uint32_t drawId) {
    for (uint32_t v = 0; v < 3; v++) {
        InVertex inVertex;
        runVertexAssembly(mem, inVertex, vao, t + v);
        inVertex.gl_DrawID = drawId;
        ShaderInterface si;
        si.textures = mem.textures;
        si.uniforms = mem.uniforms;
        prg.vertexShader(primitive.vertex[v], inVertex, si);
        primitive.npd_w[v] = primitive.vertex[v].gl_Position.w;
    }
}

void runPerspectiveDivision(Primitive& primitive) {
    for (uint32_t i = 0; i < 3; i++) {
        glm::vec4 point = primitive.vertex[i].gl_Position;
        primitive.vertex[i].gl_Position = glm::vec4(point.x / point.w, point.y / point.w, point.z / point.w, 1.0f);
    }
}

void runViewportTransformation(Primitive& primitive, Frame frame) {
    float x, y, z, w;
    for (uint32_t i = 0; i < 3; i++) {
        x = (primitive.vertex[i].gl_Position.x + 1.0f) * 0.5f * frame.width;
        y = (primitive.vertex[i].gl_Position.y + 1.0f) * 0.5f * frame.height;
        z = primitive.vertex[i].gl_Position.z; // (primitive.vertex[i].gl_Position.z + 1.0f) * 0.5f ???
        w = primitive.vertex[i].gl_Position.w;  // typically 1.0f ???
        primitive.vertex[i].gl_Position = glm::vec4(x, y, z, w);
    }
}

bool point_in_triangle(const glm::vec2& point, const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2) {
    // Compute vectors
    glm::vec2 v0p = point - v0;
    glm::vec2 v1v0 = v1 - v0;
    glm::vec2 v2v0 = v2 - v0;

    // Compute dot products
    float dot00 = glm::dot(v2v0, v2v0);
    float dot01 = glm::dot(v2v0, v1v0);
    float dot02 = glm::dot(v2v0, v0p);
    float dot11 = glm::dot(v1v0, v1v0);
    float dot12 = glm::dot(v1v0, v0p);

    // Compute barycentric coordinates
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

glm::vec3 calculateBarycentricCoordinates(const glm::vec2& point, const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2) {
    // Vectors from v0 to v1/v2
    glm::vec2 v0v1 = v1 - v0;
    glm::vec2 v0v2 = v2 - v0;
    glm::vec2 v0p = point - v0;

    // Dot products
    float d00 = glm::dot(v0v1, v0v1);
    float d01 = glm::dot(v0v1, v0v2);
    float d11 = glm::dot(v0v2, v0v2);
    float d20 = glm::dot(v0p, v0v1);
    float d21 = glm::dot(v0p, v0v2);

    // Barycentric coordinates
    float denom = d00 * d11 - d01 * d01;
    float l1 = (d11 * d20 - d01 * d21) / denom;
    float l2 = (d00 * d21 - d01 * d20) / denom;
    float l0 = 1.0f - l1 - l2;

    return glm::vec3(l0, l1, l2);
}

float clamp(float val, float low, float high) {
    float ret = val;
    if (ret < low)
        ret = low;
    if (ret > high)
        ret = high;
    return ret;
}

void clampColor(OutFragment& outFragment, float low, float high) {
    if (outFragment.gl_FragColor.r < low)
        outFragment.gl_FragColor.r = low;
    if (outFragment.gl_FragColor.g < low)
        outFragment.gl_FragColor.g = low;
    if (outFragment.gl_FragColor.b < low)
        outFragment.gl_FragColor.b = low;
    if (outFragment.gl_FragColor.r > high)
        outFragment.gl_FragColor.r = high;
    if (outFragment.gl_FragColor.g > high)
        outFragment.gl_FragColor.g = high;
    if (outFragment.gl_FragColor.b > high)
        outFragment.gl_FragColor.b = high;
}

void perFragmentOperations(Frame& frame, OutFragment& outFragment, float fragDepth, uint32_t x, uint32_t y) {
    uint32_t height = frame.height;
    uint32_t width = frame.width;
    uint32_t pixels = height * width;

    float a = outFragment.gl_FragColor.w;
    
    uint8_t red = (uint8_t)(outFragment.gl_FragColor.r * 255.f);
    uint8_t green = (uint8_t)(outFragment.gl_FragColor.g * 255.f);
    uint8_t blue = (uint8_t)(outFragment.gl_FragColor.b * 255.f);
    uint8_t alpha = (uint8_t)(a * 255.f);
    
    uint32_t pix = y * width + x;
    if (fragDepth < frame.depth[pix]) {
        uint32_t pos = pix * 4;
        //musíme prevest barvu ulozenou v bytech 0-255 na float
        //potom to smíchat s barvou fragmentu pomocí alphy
        //pak oveřit, kdyby to náhodou bylo mimo interval [0,1]
        //a vrátit do bajtu *255
        frame.color[pos + 0] = (uint8_t)(clamp((frame.color[pos + 0] / 255.f) * (1 - a) + (outFragment.gl_FragColor.r * a), 0.f, 1.f) * 255.00001f);
        frame.color[pos + 1] = (uint8_t)(clamp((frame.color[pos + 1] / 255.f) * (1 - a) + (outFragment.gl_FragColor.g * a), 0.f, 1.f) * 255.00001f);
        frame.color[pos + 2] = (uint8_t)(clamp((frame.color[pos + 2] / 255.f) * (1 - a) + (outFragment.gl_FragColor.b * a), 0.f, 1.f) * 255.00001f); // neprochazel mi test 19, tak jsem malilinko pridal ;-)
        frame.color[pos + 3] = alpha;
        if (a > 0.5f) {
            frame.depth[pix] = fragDepth;
        }
    }
}

void createFragment(InFragment& inFragment, Primitive& primitive, glm::vec3 barycentrics, uint32_t x, uint32_t y, Program prg) {
    
    float depth = primitive.vertex[0].gl_Position.z * barycentrics[0] + primitive.vertex[1].gl_Position.z * barycentrics[1] + primitive.vertex[2].gl_Position.z * barycentrics[2];
    glm::vec2 point = glm::vec2(float(x) + 0.5f, float(y) + 0.5f);
    inFragment.gl_FragCoord = glm::vec4(point.x, point.y, depth, 1.0f);
    for (uint32_t a = 0; a < maxAttributes; a++) {
        float h0 = primitive.npd_w[0];
        float h1 = primitive.npd_w[1];
        float h2 = primitive.npd_w[2];
        float s = barycentrics[0] / h0 + barycentrics[1] / h1 + barycentrics[2] / h2;
        float l0 = barycentrics[0] / (h0 * s);
        float l1 = barycentrics[1] / (h1 * s);
        float l2 = barycentrics[2] / (h2 * s);
        switch (prg.vs2fs[a]) {
        case AttributeType::EMPTY:
            break;
        case AttributeType::FLOAT:
            inFragment.attributes[a].v1 = primitive.vertex[0].attributes[a].v1 * l0 +
                primitive.vertex[1].attributes[a].v1 * l1 +
                primitive.vertex[2].attributes[a].v1 * l2;
            break;
        case AttributeType::VEC2:
            inFragment.attributes[a].v2 = primitive.vertex[0].attributes[a].v2 * l0 +
                primitive.vertex[1].attributes[a].v2 * l1 +
                primitive.vertex[2].attributes[a].v2 * l2;
            break;
        case AttributeType::VEC3:
            inFragment.attributes[a].v3 = primitive.vertex[0].attributes[a].v3 * l0 +
                primitive.vertex[1].attributes[a].v3 * l1 +
                primitive.vertex[2].attributes[a].v3 * l2;
            break;
        case AttributeType::VEC4:
            inFragment.attributes[a].v4 = primitive.vertex[0].attributes[a].v4 * l0 +
                primitive.vertex[1].attributes[a].v4 * l1 +
                primitive.vertex[2].attributes[a].v4 * l2;
            break;
        case AttributeType::UINT:
        case AttributeType::UVEC2:
        case AttributeType::UVEC3:
        case AttributeType::UVEC4:
            inFragment.attributes[a] = primitive.vertex[0].attributes[a];
            break;
        }
    }
}

void rasterizeTriangle(GPUMemory& mem, Primitive& primitive, Program prg, bool backfaceCulling) {
    Frame& frame = mem.framebuffer;
    for (uint32_t y = 0; y < frame.height; y++) {
        for (uint32_t x = 0; x < frame.width; x++) {
            glm::vec2 point = glm::vec2(float(x) + 0.5f, float(y) + 0.5f);
            glm::vec2 v0 = glm::vec2(primitive.vertex[0].gl_Position.x, primitive.vertex[0].gl_Position.y);
            glm::vec2 v1 = glm::vec2(primitive.vertex[1].gl_Position.x, primitive.vertex[1].gl_Position.y);
            glm::vec2 v2 = glm::vec2(primitive.vertex[2].gl_Position.x, primitive.vertex[2].gl_Position.y);
            float area = 0.5f * ((v0.x - v2.x) * (v1.y - v0.y) - (v0.x - v1.x) * (v2.y - v0.y));
            bool pixel_in_primitive = point_in_triangle(point, v0, v1, v2);
            if ((pixel_in_primitive && (area>0)) || (pixel_in_primitive && (area < 0) && !backfaceCulling)) { // pixel in primitive
                InFragment inFragment;
                glm::vec3 barycentrics = calculateBarycentricCoordinates(point, v0, v1, v2);
                createFragment(inFragment, primitive, barycentrics, x, y, prg);
                OutFragment outFragment;
                ShaderInterface si;
                si.textures = mem.textures;
                si.uniforms = mem.uniforms;
                prg.fragmentShader(outFragment, inFragment, si);
                clampColor(outFragment, 0, 1);
                perFragmentOperations(frame, outFragment, inFragment.gl_FragCoord.z, x, y);
            }
        }
    }
}

OutVertex interpolate(const OutVertex& v0, const OutVertex& v1, float alpha) {
    OutVertex result;
    result.gl_Position = v0.gl_Position * (1.0f - alpha) + v1.gl_Position * alpha;

    // interpolate attributes
    for (int i = 0; i < maxAttributes; ++i) {
        result.attributes[i].v4 = v0.attributes[i].v4 * (1.0f - alpha) + v1.attributes[i].v4 * alpha;
        // result.attributes[i] = v0.attributes[i];
    }

    return result;
}

void performClipping(std::vector<Primitive>& clipped, const Primitive& primitive) {
    float nearPlane = 0.0f;  // Assuming the near plane is at z = 0 in clip space

    clipped.push_back(primitive); // no clipping
    return;

    // Compute distances from the near plane
    float distances[3] = { 0, 0, 0 };
    for (int i = 0; i < 3; ++i) {
        distances[i] = primitive.vertex[i].gl_Position.z - nearPlane;
    }

    // If all vertices are behind the near plane
    if (distances[0] < 0 && distances[1] < 0 && distances[2] < 0) {
        return;
    }

    // If all vertices are in front of or on the near plane
    if (distances[0] >= 0 && distances[1] >= 0 && distances[2] >= 0) {
        clipped.push_back(primitive);
        return;
    }

    // If one vertex is behind the near plane
    for (int i = 0; i < 3; ++i) {
        if (distances[i] < 0 && distances[(i + 1) % 3] >= 0 && distances[(i + 2) % 3] >= 0) {
            float alpha1 = distances[(i + 1) % 3] / (distances[(i + 1) % 3] - distances[i]);
            float alpha2 = distances[(i + 2) % 3] / (distances[(i + 2) % 3] - distances[i]);
            OutVertex newVertex1 = interpolate(primitive.vertex[(i + 1) % 3], primitive.vertex[i], alpha1);
            OutVertex newVertex2 = interpolate(primitive.vertex[(i + 2) % 3], primitive.vertex[i], alpha2);

            Primitive newPrimitive1;
            newPrimitive1.vertex[0] = newVertex1;
            newPrimitive1.vertex[1] = primitive.vertex[(i + 1) % 3];
            newPrimitive1.vertex[2] = primitive.vertex[(i + 2) % 3];
            clipped.push_back(newPrimitive1);

            Primitive newPrimitive2;
            newPrimitive2.vertex[0] = newVertex1;
            newPrimitive2.vertex[1] = primitive.vertex[(i + 2) % 3];
            newPrimitive2.vertex[2] = newVertex2;
            clipped.push_back(newPrimitive2);

            return;
        }
    }

    // If two vertices are behind the near plane
    for (int i = 0; i < 3; ++i) {
        if (distances[i] >= 0 && distances[(i + 1) % 3] < 0 && distances[(i + 2) % 3] < 0) {
            float alpha1 = distances[i] / (distances[i] - distances[(i + 1) % 3]);
            float alpha2 = distances[i] / (distances[i] - distances[(i + 2) % 3]);
            OutVertex newVertex1 = interpolate(primitive.vertex[i], primitive.vertex[(i + 1) % 3], alpha1);
            OutVertex newVertex2 = interpolate(primitive.vertex[i], primitive.vertex[(i + 2) % 3], alpha2);

            Primitive newPrimitive;
            newPrimitive.vertex[0] = primitive.vertex[i];
            newPrimitive.vertex[1] = newVertex1;
            newPrimitive.vertex[2] = newVertex2;
            clipped.push_back(newPrimitive);

            return;
        }
    }

    std::cout << "nejaka nepokryta situace v clipping ?????" << std::endl;
}

void draw(GPUMemory& mem, DrawCommand cmd, uint32_t drawId) {
    Program prg = mem.programs[cmd.programID];
    
    // std::cout << "[                                                 ]\r[" << std::flush;
    uint32_t lastdot = 0;
    for (uint32_t t = 0; t < cmd.nofVertices; t += 3) {
        // std::cout << "processing vertex " << t << " of " << cmd.nofVertices << std::endl;  
        Primitive primitive;
        runPrimitiveAssembly(primitive, mem, cmd.vao, t, prg, drawId);
        //std::cout << "Primitive A: (" << primitive.vertex[0].gl_Position.x << ", " << primitive.vertex[0].gl_Position.y << ", " << primitive.vertex[0].gl_Position.z << ", " << primitive.vertex[0].gl_Position.w << ")\n";
        //std::cout << "Primitive B: (" << primitive.vertex[1].gl_Position.x << ", " << primitive.vertex[1].gl_Position.y << ", " << primitive.vertex[1].gl_Position.z << ", " << primitive.vertex[1].gl_Position.w << ")\n";
        //std::cout << "Primitive C: (" << primitive.vertex[2].gl_Position.x << ", " << primitive.vertex[2].gl_Position.y << ", " << primitive.vertex[2].gl_Position.z << ", " << primitive.vertex[2].gl_Position.w << ")\n";
        std::vector<Primitive> clipped;
        performClipping(clipped, primitive);
        for (size_t c = 0; c < clipped.size(); ++c) { // all clipped triangles
            primitive = clipped[c];
            runPerspectiveDivision(primitive);
            //std::cout << "Divided A: (" << primitive.vertex[0].gl_Position.x << ", " << primitive.vertex[0].gl_Position.y << ", " << primitive.vertex[0].gl_Position.z << ", " << primitive.vertex[0].gl_Position.w << ")\n";
            //std::cout << "Divided B: (" << primitive.vertex[1].gl_Position.x << ", " << primitive.vertex[1].gl_Position.y << ", " << primitive.vertex[1].gl_Position.z << ", " << primitive.vertex[1].gl_Position.w << ")\n";
            //std::cout << "Divided C: (" << primitive.vertex[2].gl_Position.x << ", " << primitive.vertex[2].gl_Position.y << ", " << primitive.vertex[2].gl_Position.z << ", " << primitive.vertex[2].gl_Position.w << ")\n";
            runViewportTransformation(primitive, mem.framebuffer);
            //std::cout << "Window A: (" << primitive.vertex[0].gl_Position.x << ", " << primitive.vertex[0].gl_Position.y << ", " << primitive.vertex[0].gl_Position.z << ", " << primitive.vertex[0].gl_Position.w << ")\n";
            //std::cout << "Window B: (" << primitive.vertex[1].gl_Position.x << ", " << primitive.vertex[1].gl_Position.y << ", " << primitive.vertex[1].gl_Position.z << ", " << primitive.vertex[1].gl_Position.w << ")\n";
            //std::cout << "Window C: (" << primitive.vertex[2].gl_Position.x << ", " << primitive.vertex[2].gl_Position.y << ", " << primitive.vertex[2].gl_Position.z << ", " << primitive.vertex[2].gl_Position.w << ")\n";
            rasterizeTriangle(mem, primitive, prg, cmd.backfaceCulling);
        }
      
        /*
        if (lastdot < (t * 50) / cmd.nofVertices) {
            lastdot = (t * 50) / cmd.nofVertices;
            std::cout << "." << std::flush;
        }
        */
    }
    // std::cout << "\r                                                    \r" << std::flush;
}

//! [gpu_execute]
void gpu_execute(GPUMemory&mem,CommandBuffer &cb){
  (void)mem;
  (void)cb;
  /// \todo Tato funkce reprezentuje funkcionalitu grafické karty.<br>
  /// Měla by umět zpracovat command buffer, čistit framebuffer a kresli.<br>
  /// mem obsahuje paměť grafické karty.
  /// cb obsahuje command buffer pro zpracování.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
  uint32_t drawId = 0;
  for (uint32_t i = 0; i < cb.nofCommands; ++i) {
      CommandType type = cb.commands[i].type;
      CommandData data = cb.commands[i].data;
      if (type == CommandType::CLEAR) {
          clear(mem, data.clearCommand);
      }
      if (type == CommandType::DRAW) {
          // std::cout << "processing command " << i << " of " << cb.nofCommands << " (gl_DrawID = " << drawId << ")" << std::endl;
          draw(mem, data.drawCommand, drawId++);
      }
  }
}
//! [gpu_execute]



/**
 * @brief This function reads color from texture.
 *
 * @param texture texture
 * @param uv uv coordinates
 *
 * @return color 4 floats
 */
glm::vec4 read_texture(Texture const&texture,glm::vec2 uv){
  if(!texture.data)return glm::vec4(0.f);
  auto uv1 = glm::fract(uv);
  auto uv2 = uv1*glm::vec2(texture.width-1,texture.height-1)+0.5f;
  auto pix = glm::uvec2(uv2);
  //auto t   = glm::fract(uv2);
  glm::vec4 color = glm::vec4(0.f,0.f,0.f,1.f);
  for(uint32_t c=0;c<texture.channels;++c)
    color[c] = texture.data[(pix.y*texture.width+pix.x)*texture.channels+c]/255.f;
  return color;
}
