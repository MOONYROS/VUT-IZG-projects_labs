/*!
 * @file
 * @brief This file contains functions for model rendering
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */
#include <student/drawModel.hpp>
#include <student/gpu.hpp>
//#include <iostream>

///\endcond


void prepareNode(GPUMemory& mem, CommandBuffer& cb, Node const& node, Model const& model, glm::mat4 const& prubeznaMatice, uint32_t& cmdID) {
    if (node.mesh >= 0) { // ma tento node mesh?
        Mesh const& mesh = model.meshes[node.mesh];

        VertexArray vao;
        vao.indexBufferID = mesh.indexBufferID;
        vao.indexOffset = mesh.indexOffset;
        vao.indexType = mesh.indexType;
        vao.vertexAttrib[0] = mesh.position;
        vao.vertexAttrib[1] = mesh.normal;
        vao.vertexAttrib[2] = mesh.texCoord;
        bool backFaceCulling = !mesh.doubleSided;

        pushDrawCommand(cb, mesh.nofIndices, 0, vao, backFaceCulling);

        auto& modelMatrixUniform = mem.uniforms[10 + cmdID * 5].m4;
        auto& inverseModelMatrixUniform = mem.uniforms[10 + cmdID * 5 + 1].m4;
        auto& diffuseColorUniform = mem.uniforms[10 + cmdID * 5 + 2].v4;
        auto& diffuseTextureIDUniform = mem.uniforms[10 + cmdID * 5 + 3].u1;
        auto& doubleSidedUniform = mem.uniforms[10 + cmdID * 5 + 4].v1;

        diffuseColorUniform = mesh.diffuseColor;
        doubleSidedUniform = (float)mesh.doubleSided;
        diffuseTextureIDUniform = mesh.diffuseTexture;
        //isTextureUniform          = mesh.diffuseTexture<0?0.f:1.f;
        modelMatrixUniform = prubeznaMatice * node.modelMatrix;
        inverseModelMatrixUniform = glm::transpose(glm::inverse(modelMatrixUniform));

        cmdID++;
    }
    for (size_t i = 0; i < node.children.size(); ++i)
        prepareNode(mem, cb, node.children[i], model, prubeznaMatice*node.modelMatrix, cmdID); // rekurze
}

/**
 * @brief This function prepares model into memory and creates command buffer
 *
 * @param mem gpu memory
 * @param commandBuffer command buffer
 * @param model model structure
 */
//! [drawModel]
void prepareModel(GPUMemory&mem,CommandBuffer&commandBuffer,Model const&model){
    (void)mem;
    (void)commandBuffer;
    (void)model;
    /// \todo Tato funkce připraví command buffer pro model a nastaví správně pamět grafické karty.<br>
    /// Vaším úkolem je správně projít model a vložit vykreslovací příkazy do commandBufferu.
    /// Zároveň musíte vložit do paměti textury, buffery a uniformní proměnné, které buffer command buffer využívat.
    /// Bližší informace jsou uvedeny na hlavní stránce dokumentace a v testech.

    for (size_t i = 0; i < model.buffers.size(); ++i)
        mem.buffers[i] = model.buffers[i];

    for (size_t i = 0; i < model.textures.size(); ++i)
        mem.textures[i] = model.textures[i];

    mem.programs[0].vertexShader = drawModel_vertexShader;
    mem.programs[0].fragmentShader = drawModel_fragmentShader;
    mem.programs[0].vs2fs[0] = AttributeType::VEC3;
    mem.programs[0].vs2fs[1] = AttributeType::VEC3;
    mem.programs[0].vs2fs[2] = AttributeType::VEC2;
    mem.programs[0].vs2fs[3] = AttributeType::UINT;

    pushClearCommand(commandBuffer, glm::vec4(.1, .15, .1, 1));

    glm::mat4 jednotkovaMatrice = glm::mat4(1.f);
    uint32_t cmdID = 0;
    for (size_t i = 0; i < model.roots.size(); ++i) {
        prepareNode(mem, commandBuffer, model.roots[i], model, jednotkovaMatrice, cmdID);
    }
} 
//! [drawModel]

/**
 * @brief This function represents vertex shader of texture rendering method.
 *
 * @param outVertex output vertex
 * @param inVertex input vertex
 * @param si shader interface
 */
//! [drawModel_vs]
void drawModel_vertexShader(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&si){
  (void)outVertex;
  (void)inVertex;
  (void)si;
  /// \todo Tato funkce reprezentujte vertex shader.<br>
  /// Vaším úkolem je správně trasnformovat vrcholy modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
  auto const& pozice = inVertex.attributes[0].v3;
  auto const& pos4 = glm::vec4(pozice, 1.f);
  auto const& nor = inVertex.attributes[1].v3;
  auto const& coord = inVertex.attributes[2].v2;
  auto const& viewProjectionMatrix = si.uniforms[0].m4;
  auto const& modelMatrix = si.uniforms[10 + inVertex.gl_DrawID * 5 + 0].m4;
  auto const& invModelMatrix = si.uniforms[10 + inVertex.gl_DrawID * 5 + 1].m4;

  outVertex.gl_Position = viewProjectionMatrix * modelMatrix * pos4;
  outVertex.attributes[0].v3 = glm::vec3(modelMatrix * pos4);
  outVertex.attributes[1].v3 = glm::vec3(invModelMatrix * glm::vec4(nor, 0.f));
  outVertex.attributes[2].v2 = coord;
  outVertex.attributes[3].u1 = inVertex.gl_DrawID;
}
//! [drawModel_vs]

/**
 * @brief This functionrepresents fragment shader of texture rendering method.
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param si shader interface
 */
//! [drawModel_fs]
void drawModel_fragmentShader(OutFragment&outFragment,InFragment const&inFragment,ShaderInterface const&si){
  (void)outFragment;
  (void)inFragment;
  (void)si;
  /// \todo Tato funkce reprezentujte fragment shader.<br>
  /// Vaším úkolem je správně obarvit fragmenty a osvětlit je pomocí lambertova osvětlovacího modelu.
  /// Bližší informace jsou uvedeny na hlavní stránce dokumentace.
  auto const& pos = inFragment.attributes[0].v3;
  auto nor = inFragment.attributes[1].v3;
  auto const& coord = inFragment.attributes[2].v2;
  auto const& gl_DrawID = inFragment.attributes[3].u1;

  auto const& lightPosition = si.uniforms[1].v3;
  auto const& cameraPosition = si.uniforms[2].v3;
  auto const& materialDiffuseColor = si.uniforms[10 + gl_DrawID * 5 + 2].v4; // difuzní barva materialu, -pokud není textura
  auto const& textureID = si.uniforms[10 + gl_DrawID * 5 + 3].i1; // číslo textury nebo - 1 pokud textura není.
  auto const& doubleSided = si.uniforms[10 + gl_DrawID * 5 + 4].v1; // 0.f znamená, že je to jednostraný povrch, 1.f znamená, že je to doubleSided
  
  float ambientFactor = 0.2f;

  // auto nor = nor2;
  if (doubleSided > 0.f)
      nor *= glm::sign(glm::dot(cameraPosition - pos, nor));

  glm::vec4 diffuseColor;
  if (textureID < 0)
      diffuseColor = materialDiffuseColor;
  else
      diffuseColor = read_texture(si.textures[textureID], coord);

  auto L = glm::normalize(lightPosition - pos);
  float diffuseFactor = glm::max(glm::dot(L, glm::normalize(nor)), 0.f);

  glm::vec3 dL = glm::vec3(diffuseColor) * diffuseFactor;
  glm::vec3 aL = glm::vec3(diffuseColor) * ambientFactor;

  outFragment.gl_FragColor = glm::vec4(aL + dL, diffuseColor.a);
}
//! [drawModel_fs]

