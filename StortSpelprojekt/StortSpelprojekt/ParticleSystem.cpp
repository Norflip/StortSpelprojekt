#include "stdafx.h"
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(Object* object, Shader* shader)
{
	//this->object = new Object("particles");
	this->particlesShader = shader;
	//this->particlesMaterial = new Material(particlesShader);
	this->object = object;
	//this->camera = camera;

	/* Default stuff about every particle */
	this->differenceOnX = 0.0f;
	this->differenceOnY = 0.0f;
	this->differenceOnZ = 0.0f;
	this->particleVelocity = 0.0f;
	this->particleVelocityVariation = 0.0f;
	this->particleSize = 0.0f;
	this->maxParticles = 0;

	this->currentParticleCount = 0;
	this->accumulatedTime = 0.0f;

	this->vertexCount = 0;
	this->indexCount = 0;

	this->vertices = 0;
	this->vertexBuffer = 0;
	this->indexBuffer = 0;

	this->particleList = 0;
	this->texture = 0;

	this->worldmatrix = dx::XMMatrixIdentity();
}

ParticleSystem::ParticleSystem(const ParticleSystem& other)
{
	this->differenceOnX = other.differenceOnX;
	this->differenceOnY = other.differenceOnY;
	this->differenceOnZ = other.differenceOnZ;
	this->particleVelocity = other.particleVelocity;
	this->particleVelocityVariation = other.particleVelocityVariation;
	this->particleSize = other.particleSize;
	this->maxParticles = other.maxParticles;

	this->currentParticleCount = other.currentParticleCount;
	this->accumulatedTime = other.accumulatedTime;

	this->vertexCount = other.vertexCount;
	this->indexCount = other.indexCount;

	this->vertices = other.vertices;
	this->vertexBuffer = other.vertexBuffer;
	this->indexBuffer = other.indexBuffer;

	this->particleList = other.particleList;
	this->texture = other.texture;

	this->worldmatrix = other.worldmatrix;
}

ParticleSystem::~ParticleSystem()
{
	//Shutdown();
}

void ParticleSystem::InitializeParticles(ID3D11Device* device, LPCWSTR textureFilename)
{	
	/*
		Deviation on each axis. (A random deviation). Is from the point on the scene each particle will be spawned and (random -2 to 2 in x-axis and z-axis).
		Speed (Velocity) for each particle. + Variation makes it between 9.8f - 10.2f
		Particlesize is based on the created quad.
	*/

	differenceOnX = 0.2f;
	differenceOnY = 0.2f;
	differenceOnZ = 0.2f;

	particleVelocity = 1.0f;
	particleVelocityVariation = 0.2f;

	particleSize = 0.05f;


	/*
		Set a maximum amount of particles allowed at the same time in the scene.
		Create the list of particles.
		Init all of them to false (not active on the scene.)
	*/

	maxParticles = 10;
	particleList = new Particles[maxParticles];	

	for (int i = 0; i < maxParticles; i++) {
		particleList[i].active = false;
	}

	/*
		Currentparticlecount is the current particles falling down in the scene at the moment. (0 at the beginning)
		Accumulated time is the time that increments based on deltatime. When a certain time is reached, a new particle is created.
	*/
	currentParticleCount = 0;
	accumulatedTime = 0.0f;


	/*
		Initialize vertex and index buffer.
		Vertex buffer is a dynamic buffer beacause of the particle system.
		Particles are temporary at the scene, so we want our buffer to be a dynamic buffer so we can update the amount of particles created and deleted
		on the scene dynamicly from the cpu and not from our shader.
	*/

	InitializeBuffers(device);
	LoadTexture(device, textureFilename);	
}

void ParticleSystem::Shutdown()
{
	// Release the index buffer.
	if (indexBuffer)
	{
		indexBuffer->Release();
		indexBuffer = 0;
	}

	// Release the vertex buffer.
	if (vertexBuffer)
	{
		vertexBuffer->Release();
		vertexBuffer = 0;
	}

	// Release the particle list.
	if (particleList) {
		delete[] particleList;
		particleList = 0;
	}
	
	if (vertices) {
		delete vertices;
		vertices = 0;
	}
}

void ParticleSystem::Update(float frameTime, CameraComponent* camera, ID3D11DeviceContext* context)
{	

	/* Update particle-list dynamiclly */
	dx::XMFLOAT3 particlesPosition;

	/* Set billboardplane position */
	particlesPosition.x = object->GetTransform().GetPosition().m128_f32[0];
	particlesPosition.y = object->GetTransform().GetPosition().m128_f32[1] +0.5f;
	particlesPosition.z = object->GetTransform().GetPosition().m128_f32[2];

	/* Set angle for particles, "look at cameraposition all the time" */
	/* Defines the angle in radians between two vectors * 1 degrees to give our xmatrixrotationy an degree value*/
	dx::XMVECTOR v = camera->GetOwner()->GetTransform().GetPosition();
	float xPos = v.m128_f32[0];
	float zPos = v.m128_f32[2];
	double anglepart = atan2(particlesPosition.x - xPos, particlesPosition.z - zPos) * (180.0 / dx::XM_PI);
	float rotationPart = (float)anglepart * 0.0174532925f;

	dx::XMMATRIX worldParticles = dx::XMMatrixIdentity();
	dx::XMMATRIX particlesRotationY = dx::XMMatrixRotationY(rotationPart);
	dx::XMMATRIX particlesTranslation = dx::XMMatrixTranslation(particlesPosition.x, particlesPosition.y, particlesPosition.z);
	worldParticles = particlesRotationY * particlesTranslation;
	SetWorldMatrix(worldParticles);

	/*
		Delete old particles that reaches a height. 
		Create new particles after some time.
		Update the created particles so they fall from a centerposition to a -y position until they disappear.

		Update the dynamic vertexbuffer with the new position of each particle
	*/

	DeleteParticles();	
	CreateParticle(frameTime);
	UpdateParticles(frameTime);

	// Update the dynamic vertex buffer with the new position of each particle.
	UpdateBuffers(context);
}

void ParticleSystem::Render(ID3D11DeviceContext* context, CameraComponent* camera)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(Vertex);
	offset = 0;
	
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	SetCBuffers(context, camera);
	particlesShader->BindToContext(context);
	context->DrawIndexed(indexCount, 0, 0);
}

void ParticleSystem::SetDaviation(float x, float y, float z)
{
	this->differenceOnX = x;
	this->differenceOnY = y;
	this->differenceOnZ = z;
}

void ParticleSystem::SetWorldMatrix(dx::XMMATRIX worldmatrix)
{
	this->worldmatrix = worldmatrix;
}

dx::XMMATRIX ParticleSystem::GetWorldMatrix()
{
	return this->worldmatrix;
}

int ParticleSystem::GetIndexCount()
{
	return indexCount;
}

void ParticleSystem::LoadTexture(ID3D11Device* device, LPCWSTR textureFilename)
{
	hr = dx::CreateWICTextureFromFile(device, textureFilename, nullptr, &srv);
	if (FAILED(hr))
		MessageBox(0, L"Failed to 'Load WIC Texture'", L"Graphics scene Initialization Message", MB_ICONERROR);

	texture.SetShaderResourceView(srv);	

	//particlesMaterial->SetTexture(texture, TEXTURE_DIFFUSE_SLOT, ShaderBindFlag::PIXEL);
	//particlesMaterial->SetSampler(DXHelper::CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, device), 0, ShaderBindFlag::PIXEL);

	samplerState = DXHelper::CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, device);
}

void ParticleSystem::InitializeBuffers(ID3D11Device* device)
{
	unsigned long* indices;
	HRESULT result;

	vertexCount = maxParticles * 6;
	indexCount = vertexCount;

	// Create the vertex array for the particles that will be rendered.
	vertices = new Vertex[vertexCount];
	
	// Create the index array.
	indices = new unsigned long[indexCount];

	for (int i = 0; i < indexCount; i++) {
		indices[i] = i;
	}

	// Set up the description of the dynamic vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
	assert(SUCCEEDED(result));


	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
	assert(SUCCEEDED(result));

	// Release the index array since it is no longer needed.
	delete[] indices;
	indices = 0;




	/* * * * * * * * CBuffer stuffy stuff * * * * * * * * */
	D3D11_BUFFER_DESC cBufferDescription;

	// Setup the description of the bufferPerObject constant buffer that is in the vertex shader.
	ZeroMemory(&cBufferDescription, sizeof(D3D11_BUFFER_DESC));
	cBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	cBufferDescription.ByteWidth = static_cast<uint32_t>(sizeof(cBufferPerObjectParticles) + (16 - (sizeof(cBufferPerObjectParticles) % 16)));
	cBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDescription.CPUAccessFlags = 0;
	cBufferDescription.MiscFlags = 0;
	cBufferDescription.StructureByteStride = 0;

	result = device->CreateBuffer(&cBufferDescription, NULL, &bufferPerObjectParticles);
	assert(SUCCEEDED(result));
}

void ParticleSystem::SetCBuffers(ID3D11DeviceContext* context, CameraComponent* camera)
{
	dx::XMMATRIX WVP = worldmatrix * camera->GetViewMatrix() * camera->GetProjectionMatrix();
	cbPerObjectParticles.worldViewProj =  DirectX::XMMatrixTranspose(WVP);
	cbPerObjectParticles.world = DirectX::XMMatrixTranspose(worldmatrix);

	context->UpdateSubresource(bufferPerObjectParticles, 0, nullptr, &cbPerObjectParticles, 0, 0);
	context->VSSetConstantBuffers(0, 1, &bufferPerObjectParticles);

	// Set shader texture resource in the pixel shader.
	context->PSSetShaderResources(0, 1, &srv);
	context->PSSetSamplers(0, 1, &samplerState);
}

void ParticleSystem::CreateParticle(float frameTime)
{
	// Increment the frame time.
	accumulatedTime += frameTime;

	// Set emit particle to false for now.
	bool emitParticle = false;

	// Check if it is time to emit a new particle or not.
	if (accumulatedTime > 0.1f) {
		accumulatedTime = 0.0f;
		emitParticle = true;
	}

	// If there are particles to emit then emit one per frame.
	if ((emitParticle == true) && (currentParticleCount < (maxParticles - 1)))
	{
		currentParticleCount++;

		/*
			Generate the randomized position based on the deviation on each axis.
			Generate the randomized velocity based on the deviation on each axis.
			Red green and blue are set to 1,1,1 because we are using a texture as color.
		*/

		float positionX =  (((float)rand() - (float)rand()) / RAND_MAX)* differenceOnX;
		float positionY =  (((float)rand() - (float)rand()) / RAND_MAX)* differenceOnY;
		float positionZ =  (((float)rand() - (float)rand()) / RAND_MAX)* differenceOnZ;

		float velocity = particleVelocity + (((float)rand() - (float)rand()) / RAND_MAX) * particleVelocityVariation;

		float red = 1.0f;
		float green = 1.0f;
		float blue = 1.0f;

		/*
			Sort the listarray and find a place for the new particle.
			if a particle not is active or has a different depth than the other searched particle-place
			mark that as found and copy the array over by one position from
			We fill up the new place with the one before adn so on until we reach the new index.

		*/
		int index = 0;
		bool found = false;
		while (!found)
		{
			if ((particleList[index].active == false) || (particleList[index].posz < positionZ))
			{
				found = true;
			}
			else
			{
				index++;
			}
		}

		/*
			Sort the list because we have a new currentParticleCount, move everything forwards until we reach the new place
		*/
		int i = currentParticleCount;
		int j = i - 1;
		while (i != index)
		{
			particleList[i].posx = particleList[j].posx;
			particleList[i].posy = particleList[j].posy;
			particleList[i].posz = particleList[j].posz;
			particleList[i].red = particleList[j].red;
			particleList[i].green = particleList[j].green;
			particleList[i].blue = particleList[j].blue;
			particleList[i].velocity = particleList[j].velocity;
			particleList[i].active = particleList[j].active;
			i--;
			j--;
		}


		/*
			Insert the new particle setup on that index place
		*/

		particleList[index].posx = positionX;
		particleList[index].posy = positionY;
		particleList[index].posz = positionZ;
		particleList[index].red = red;
		particleList[index].green = green;
		particleList[index].blue = blue;
		particleList[index].velocity = velocity;
		particleList[index].active = true;
	}

}

void ParticleSystem::UpdateParticles(float frameTime)
{
	// Each frame we update all the particles by making them move downwards using their position, velocity, and the frame time.
	for (int i = 0; i < currentParticleCount; i++)
	{
		particleList[i].posy = particleList[i].posy + (particleList[i].velocity * ((float)frameTime * 0.6f));
	}
}

void ParticleSystem::DeleteParticles()
{
	/*
		Kill all the particles that have gone below a certain height range. ( -45 from their startposition )	
	*/
	for (int i = 0; i < maxParticles; i++)
	{
		/*
			If a particle is active and has reached 45 = set active to false and decrement currentParticlecount.
		*/
		if ((particleList[i].active == true) && (particleList[i].posy > 0.5f))
		{
			particleList[i].active = false;
			currentParticleCount--;

			/*
				Shift all the live particles back up the array to erase the destroyed particle and keep the array sorted correctly.
			*/
			for (int j = i; j < maxParticles - 1; j++) {
				particleList[j].posx = particleList[j + 1].posx;
				particleList[j].posy = particleList[j + 1].posy;
				particleList[j].posz = particleList[j + 1].posz;
				particleList[j].red = particleList[j + 1].red;
				particleList[j].green = particleList[j + 1].green;
				particleList[j].blue = particleList[j + 1].blue;
				particleList[j].velocity = particleList[j + 1].velocity;
				particleList[j].active = particleList[j + 1].active;
			}
		}
	}

}

bool ParticleSystem::UpdateBuffers(ID3D11DeviceContext* context)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Vertex* verticesPtr;

	/*
		Build the vertex array from the particle list array.
		Each particle is a quad made out of two triangles.
	*/

	int index = 0;

	for (int i = 0; i < currentParticleCount; i++)
	{
		// Bottom left.
		vertices[index].position = dx::XMFLOAT3(particleList[i].posx - particleSize, (particleList[i].posy) - particleSize, particleList[i].posz);
		vertices[index].texcoord = dx::XMFLOAT2(0.0f, 1.0f);
		vertices[index].color = dx::XMFLOAT4(particleList[i].red, particleList[i].green, particleList[i].blue, 1.0f);
		index++;

		// Top left.
		vertices[index].position = dx::XMFLOAT3(particleList[i].posx - particleSize, (particleList[i].posy) + particleSize, particleList[i].posz);
		vertices[index].texcoord = dx::XMFLOAT2(0.0f, 0.0f);
		vertices[index].color = dx::XMFLOAT4(particleList[i].red, particleList[i].green, particleList[i].blue, 1.0f);
		index++;

		// Bottom right.
		vertices[index].position = dx::XMFLOAT3(particleList[i].posx + particleSize, (particleList[i].posy) - particleSize, particleList[i].posz);
		vertices[index].texcoord = dx::XMFLOAT2(1.0f, 1.0f);
		vertices[index].color = dx::XMFLOAT4(particleList[i].red, particleList[i].green, particleList[i].blue, 1.0f);
		index++;

		// Bottom right.
		vertices[index].position = dx::XMFLOAT3(particleList[i].posx + particleSize, (particleList[i].posy) - particleSize, particleList[i].posz);
		vertices[index].texcoord = dx::XMFLOAT2(1.0f, 1.0f);
		vertices[index].color = dx::XMFLOAT4(particleList[i].red, particleList[i].green, particleList[i].blue, 1.0f);
		index++;

		// Top left.
		vertices[index].position = dx::XMFLOAT3(particleList[i].posx - particleSize, (particleList[i].posy) + particleSize, particleList[i].posz);
		vertices[index].texcoord = dx::XMFLOAT2(0.0f, 0.0f);
		vertices[index].color = dx::XMFLOAT4(particleList[i].red, particleList[i].green, particleList[i].blue, 1.0f);
		index++;

		// Top right.
		vertices[index].position = dx::XMFLOAT3(particleList[i].posx + particleSize, (particleList[i].posy) + particleSize, particleList[i].posz);
		vertices[index].texcoord = dx::XMFLOAT2(1.0f, 0.0f);
		vertices[index].color = dx::XMFLOAT4(particleList[i].red, particleList[i].green, particleList[i].blue, 1.0f);
		index++;
	}


	/*
		Resource is mapped for writing, the previous contents of the resource will be undefined.
		Map/Unmap is a common use with temporary buffers. - Dynamic usage of our vertexbuffer. (It is at least prefered)
	*/

	// Lock the vertex buffer.
	result = context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);	// maybe D3D11_MAP_WRITE_NO_OVERWRITE
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (Vertex*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(Vertex) * vertexCount));

	// Unlock the vertex buffer.
	context->Unmap(vertexBuffer, 0);

	return true;
}
