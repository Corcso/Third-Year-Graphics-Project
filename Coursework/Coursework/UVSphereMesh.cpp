// Sphere Mesh
// Generates a cube sphere.
#include "UVSphereMesh.h"
#include <vector>

// Store shape resolution (default is 20), initialise buffers and load texture.
UVSphereMesh::UVSphereMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution)
{
	resolution = lresolution;
	initBuffers(device);
}

// Release resources.
UVSphereMesh::~UVSphereMesh()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}

// Generate sphere. Generates a cube based on resolution provided. Then normalises vertex positions to create sphere.
// Shape has texture coordinates and normals.
// This code was written myself for the CMP203 assesment, mesh generation is not assessed for this coursework, this is being used to help showcase the PBR shader. 
void UVSphereMesh::initBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// 6 vertices per quad, res*res is face, times 6 for each face
	vertexCount = ((6 * resolution) * resolution) * 6;
	indexCount = vertexCount;

	std::vector<VertexType> verticesInOrder;
	// The sphere uses the parametric equation for the sphere to generate its points.
	// Mush like the disc it iterates over 2 variables and uses the current and next to form its shapes
	// This sphere is made of rectangles. 
	// All normals are infact the same as the vertices, this gives the sphere a accurate look under light rather than looking like a sphere made of squares.
	// UVs are calculated in a mercator fassion
	// Theta is latitude where phi is longitude
	float thetaIncrement = 2 * 3.141592f / resolution;
	float phiIncrement = 3.141592f / resolution;
	float theta = 0.000001; float phi = 0.000001; // Cheat placing the caps manually by setting the start to a very small 0, this means a small tangent is generated then normalized in the shader to be normal
	float thetaPlusInc = thetaIncrement;
	// For each ring, increasing phi
	while (true) {
		// If phi is 180 or more, we are done
		if (phi >= 3.141592f) { break; }
		// set theta back to 0
		theta = 0; thetaPlusInc = thetaIncrement;
		// for each quad in ring, increasing theta
		while (true) {
			// Pre calculations for UV
			float ySeg = (phi) / 3.141592f;
			float xSeg = ((2 * 3.141592f) - theta) / (2 * 3.141592f);
			float ySegNext = ((phi + phiIncrement)) / 3.141592f;
			float xSegNext = ((2 * 3.141592f) - (theta + thetaIncrement)) / (2 * 3.141592f);
			// bottom left
			verticesInOrder.push_back(VertexType{
				DirectX::XMFLOAT3(0.5 * cos(theta) * sin(phi), 0.5 * cos(phi), 0.5 * sin(phi) * sin(theta)),
				DirectX::XMFLOAT2(xSeg, ySeg),
				DirectX::XMFLOAT3(cos(theta) * sin(phi), cos(phi), sin(phi) * sin(theta)),
				DirectX::XMFLOAT3(sin(theta) * sin(phi), 0, -sin(phi) * cos(theta)),
				DirectX::XMFLOAT3(cos(theta) * cos(phi), -sin(phi), cos(phi) * sin(theta)) });
			// bottom right
			// If theta is back to the start 360 or more, use angle 0 instead of 360
			if (theta >= 2 * 3.141592f) { thetaPlusInc = 0; }
			// top right
			verticesInOrder.push_back(VertexType{
				DirectX::XMFLOAT3(0.5 * cos(thetaPlusInc) * sin(phi + phiIncrement), 0.5 * cos(phi + phiIncrement), 0.5 * sin(phi + phiIncrement) * sin(thetaPlusInc)),
				DirectX::XMFLOAT2(xSegNext, ySegNext),
				DirectX::XMFLOAT3(cos(thetaPlusInc) * sin(phi + phiIncrement), cos(phi + phiIncrement), sin(phi + phiIncrement) * sin(thetaPlusInc)),
				DirectX::XMFLOAT3(sin(thetaPlusInc) * sin(phi + phiIncrement), 0, -sin(phi + phiIncrement) * cos(thetaPlusInc)),
				DirectX::XMFLOAT3(cos(thetaPlusInc) * cos(phi + phiIncrement), -sin(phi + phiIncrement), cos(phi + phiIncrement) * sin(thetaPlusInc)) });

			verticesInOrder.push_back(VertexType{
				DirectX::XMFLOAT3(0.5 * cos(thetaPlusInc) * sin(phi), 0.5 * cos(phi), 0.5 * sin(phi) * sin(thetaPlusInc)),
				DirectX::XMFLOAT2(xSegNext, ySeg),
				DirectX::XMFLOAT3(cos(thetaPlusInc) * sin(phi), cos(phi), sin(phi) * sin(thetaPlusInc)),
				DirectX::XMFLOAT3(sin(thetaPlusInc) * sin(phi), 0, -sin(phi) * cos(thetaPlusInc)),
				DirectX::XMFLOAT3(cos(thetaPlusInc) * cos(phi), -sin(phi), cos(phi) * sin(thetaPlusInc)) });
			
			// top right
			verticesInOrder.push_back(VertexType{
				DirectX::XMFLOAT3(0.5 * cos(thetaPlusInc) * sin(phi + phiIncrement), 0.5 * cos(phi + phiIncrement), 0.5 * sin(phi + phiIncrement) * sin(thetaPlusInc)),
				DirectX::XMFLOAT2(xSegNext, ySegNext),
				DirectX::XMFLOAT3(cos(thetaPlusInc) * sin(phi + phiIncrement), cos(phi + phiIncrement), sin(phi + phiIncrement) * sin(thetaPlusInc)),
				DirectX::XMFLOAT3(sin(thetaPlusInc) * sin(phi + phiIncrement), 0, -sin(phi + phiIncrement) * cos(thetaPlusInc)),
				DirectX::XMFLOAT3(cos(thetaPlusInc) * cos(phi + phiIncrement), -sin(phi + phiIncrement), cos(phi + phiIncrement) * sin(thetaPlusInc)) });
			
			// bottom left
			verticesInOrder.push_back(VertexType{
				DirectX::XMFLOAT3(0.5 * cos(theta) * sin(phi), 0.5 * cos(phi), 0.5 * sin(phi) * sin(theta)),
				DirectX::XMFLOAT2(xSeg, ySeg),
				DirectX::XMFLOAT3(cos(theta) * sin(phi), cos(phi), sin(phi) * sin(theta)),
				DirectX::XMFLOAT3(sin(theta) * sin(phi), 0, -sin(phi) * cos(theta)),
				DirectX::XMFLOAT3(cos(theta) * cos(phi), -sin(phi), cos(phi) * sin(theta)) });
			// top left
			verticesInOrder.push_back(VertexType{
				DirectX::XMFLOAT3(0.5 * cos(theta) * sin(phi + phiIncrement), 0.5 * cos(phi + phiIncrement), 0.5 * sin(phi + phiIncrement) * sin(theta)),
				DirectX::XMFLOAT2(xSeg, ySegNext),
				DirectX::XMFLOAT3(cos(theta) * sin(phi + phiIncrement), cos(phi + phiIncrement), sin(phi + phiIncrement) * sin(theta)),
				DirectX::XMFLOAT3(sin(theta) * sin(phi + phiIncrement),0, -sin(phi + phiIncrement) * cos(theta)),
				DirectX::XMFLOAT3(cos(theta) * cos(phi + phiIncrement), -sin(phi + phiIncrement), cos(phi + phiIncrement) * sin(theta)) });
			
			// bottom right

			// If theta is back to the start 360 or more, finish up this ring
			if (theta >= 2 * 3.141592f) { break; }
			// increase theta and the theta plus increment
			theta += thetaIncrement;
			thetaPlusInc = theta + thetaIncrement;
		}
		phi += phiIncrement;
	}
	
	// Convert vectors to indexed arrays
	std::vector<VertexType> uniqueVertices;
	std::vector<unsigned long> indices;
	for (int i = 0; i < verticesInOrder.size(); i++) {
		bool matchFound = false;
		for (int u = 0; u < uniqueVertices.size(); u++) {
			// If exact point is already in the unique vectors then add its index to the indices array
			if (uniqueVertices[u] == verticesInOrder[i]) {
				indices.push_back(u);
				matchFound = true;
			}
		}
		// If its a new point, add it to each vector and push back the index which is the next one in the vector. 
		if (!matchFound) {
			indices.push_back(uniqueVertices.size());
			uniqueVertices.push_back(verticesInOrder[i]);
		}
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * uniqueVertices.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = uniqueVertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indices.size();
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
}

