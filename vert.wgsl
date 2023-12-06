struct VertexUniform {
    matrix: mat4x4<f32>,
}
@binding(0) @group(0) var<uniform> ubos : array<VertexUniform, 3>;

struct VertexOut {
     @builtin(position) position_clip : vec4<f32>,
     @location(0) frag_uv : vec2<f32>,
     @location(1) frag_bary: vec3<f32>,
     @interpolate(flat) @location(2) instance_index: u32,
}

@vertex fn main(
    @builtin(instance_index) instanceIdx : u32,
    @builtin(vertex_index) vertex_index: u32,
    @location(0) position: vec4<f32>,
    @location(1) uv: vec2<f32>,
) -> VertexOut {
     var output : VertexOut;
     output.position_clip = ubos[instanceIdx].matrix * position;
     output.frag_uv = uv;
     output.frag_bary = vec3<f32>(
        f32(vertex_index % 3u == 1u),
        f32(vertex_index % 3u == 2u),
        f32(vertex_index % 3u == 0u),
     );
     output.instance_index = instanceIdx;
     return output;
}
