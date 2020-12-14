#include "stdafx.h"
#include "SkeletonAni.h"

dx::SimpleMath::Matrix& SkeletonAni::Lerp(float elapsedTime, std::vector<Bone>& keys)
{
    animationTime = elapsedTime * fps;
    
    currentFrame = fmodf(animationTime, length);

    firstIndex = 0;
    secondIndex = 1;

    for (unsigned int i = 0; i < keys.size() - 1; i++) //Find what frames the timeline is currently in between.
    {
       
        if (currentFrame < keys[i + 1].frame)
        {
            firstIndex = i;
            secondIndex = i + 1;
            break;
        }


    }

   
    //lerp between the frames to find how close to each frame the animation is.

    t = (currentFrame - (float)firstIndex) / ((float)secondIndex - (float)firstIndex);

    CLAMP(t, 0, 1); //saturate the interpolation value


    if (t < 0.0f)
    {
        t = 0.0f;
    }
    else if (t > 1.0f)
    {
        t = 1.0f;
    }

    assert(t >= 0.0f && t <= 1.0f);

    
    
    dx::SimpleMath::Quaternion slerpedQ = rotQ.Slerp(keys[firstIndex].rotationQuaternion, keys[secondIndex].rotationQuaternion, t);
    
    dx::SimpleMath::Vector3 lerpedTrans = transV.Lerp(keys[firstIndex].translationVector, keys[secondIndex].translationVector, t);
    
    RT = rotQM.CreateFromQuaternion(slerpedQ) * transM.CreateTranslation(lerpedTrans);
  

   
   

    return RT;

    

}

dx::SimpleMath::Quaternion SkeletonAni::sLerpQuaternions(dx::SimpleMath::Quaternion quat, dx::SimpleMath::Quaternion quat2)
{
    dx::SimpleMath::Quaternion slerpQuat = slerpQuat.Slerp(quat, quat2, t);

    return slerpQuat;
}

void SkeletonAni::FindChildren(float elapsedTime, const DirectX::XMMATRIX& globalParent, std::vector<Bone>& keys, std::map<std::string, unsigned int>& map, int startJoint, int endJoint)
{

    for (unsigned int i = startJoint; i < endJoint + 1; i++) //recursively find all the children and repeat.
    {
        if (keyBones[i][0].parentName == keys[0].name)
        {
            FindChildren(elapsedTime, globalParent, keyBones[i], map, startJoint, endJoint);

            map.insert({ keyBones[i][0].name, i });
          //  std::cout << keyBones[i][0].name << std::endl;
        }
    }
}

SkeletonAni::SkeletonAni()
{
   
    animationTime = 0.0f;
    bones.resize(60); //Maximum number of bones, the animation doesn't need to have this many.
}

std::vector<dx::XMFLOAT4X4>& SkeletonAni::Makeglobal(float elapsedTime, const DirectX::XMMATRIX& globalParent, std::vector<Bone>& keys)
{
    DirectX::SimpleMath::Matrix toRoot = Lerp(elapsedTime, keys) * globalParent; //These matrices are local, need to make them global recursively.

    ftIndex = keys[0].index; //all of these indices have the same index number.

    DirectX::SimpleMath::Matrix finalTransform = offsetM[ftIndex] * toRoot;


    XMStoreFloat4x4(&bones[ftIndex], finalTransform.Transpose());

    for (unsigned int i = 0; i < keyBones.size(); i++) //recursively find all the children and repeat.
    {
        if (keyBones[i][0].parentName == keys[0].name)
        {
            Makeglobal(elapsedTime, toRoot, keyBones[i]);
        }
    }

    return bones;
}

std::string SkeletonAni::GetRootName()
{
    std::string rootName = "RootNode"; //This is the name given to each rootbone inside fbx sdk from me.
    std::string rootNode;

    for (unsigned int i = 0; i < keyBones.size(); i++)
    {
        if (keyBones[i][0].parentName == rootName)
        {
            rootNode = keyBones[i][0].name;
            break;
        }
    }
    return rootNode;
}

unsigned int SkeletonAni::GetNrOfBones() const
{
    return (unsigned int)offsetM.size(); //these are one per bone.
}

void SkeletonAni::SetUpOffsetsFromMatrices(std::vector<SkeletonOffsetsHeader>& offsets)
{
    offsetM.clear();
    offsetM.resize(offsets.size());
    for (unsigned int i = 0; i < offsets.size(); i++)
    {

        DirectX::SimpleMath::Matrix mat = DirectX::XMMatrixSet(offsets[i].m[0], offsets[i].m[1], offsets[i].m[2], offsets[i].m[3]
            , offsets[i].m[4], offsets[i].m[5], offsets[i].m[6], offsets[i].m[7]
            , offsets[i].m[8], offsets[i].m[9], offsets[i].m[10], offsets[i].m[11]
            , offsets[i].m[12], offsets[i].m[13], offsets[i].m[14], offsets[i].m[15]);

        /*dx::XMStoreFloat4x4(&offsetM[boneIDMap[(std::string)offsets[i].linkName]], mat);*/
        offsetM[boneIDMap[(std::string)offsets[i].linkName]] = mat;
    }
}

std::vector<Bone>* SkeletonAni::GetRootKeyJoints()
{
    bool found = false;
    std::vector<Bone>* bones = nullptr;

    for (unsigned int i = 0; i < keyBones.size() && !found; i++)
    {
        if (keyBones[i][0].name == GetRootName())
        {
            bones = &keyBones[i];
            found = true;
        }
    }

    return bones;
}

void SkeletonAni::SetUpIDMapAndFrames(std::map<std::string, unsigned int> boneIDMap, float fps, float aniLenght)
{
    this->fps = fps;
    this->length = aniLenght;
    this->boneIDMap = boneIDMap;


}

void SkeletonAni::SetUpKeys(std::string boneName, std::vector<SkeletonKeysHeader>& keys)
{
    std::vector<Bone> jointKeysVector;
    keyBones.resize(offsetM.size());
    for (unsigned int i = 0; i < keys.size(); i++)
    {
        Bone joint;

        joint.index = boneIDMap[(std::string)keys[i].linkName];
        joint.frame = keys[i].currentKeyFrameIndex;
        joint.name = (std::string)keys[i].linkName;
        joint.parentName = (std::string)keys[i].parentName;
        DirectX::SimpleMath::Quaternion rot(keys[i].r[0], keys[i].r[1], keys[i].r[2], keys[i].r[3]);// = DirectX::XMVectorSet(keys[i].r[0], keys[i].r[1], keys[i].r[2], keys[i].r[3]);
        DirectX::SimpleMath::Vector3 trans(keys[i].t[0], keys[i].t[1], keys[i].t[2]);// = DirectX::XMVectorSet(keys[i].t[0], keys[i].t[1], keys[i].t[2], keys[i].t[3]); //konverteras till left handed i fbx


       /* dx::XMStoreFloat4(&joint.rotationQuaternion, rot);
        dx::XMStoreFloat4(&joint.translationVector, trans);*/
        
        joint.rotationQuaternion = rot;
        joint.translationVector = trans;

        jointKeysVector.push_back(joint); //every joint contains all the keyframes.



    }



    for (unsigned int i = 0; i < jointKeysVector.size(); i++)
    {
        keyBones[boneIDMap[boneName]].push_back(jointKeysVector[i]);
    }



}



std::map<std::string, unsigned int>& SkeletonAni::GetBoneIDMap()
{
    return boneIDMap;
}

std::vector<std::vector<Bone>>& SkeletonAni::GetKeyFrames()
{
    return this->keyBones;
}

std::vector<dx::SimpleMath::Matrix>& SkeletonAni::GetOffsets()
{
    return this->offsetM;
}

float SkeletonAni::GetFPS()
{
    return this->fps;
}

float SkeletonAni::GetAniLength()
{
    return this->length;
}

void SkeletonAni::SetOffsetsDirect(std::vector<dx::SimpleMath::Matrix>& directOffsets)
{
    this->offsetM = directOffsets;
}

void SkeletonAni::SetKeyFramesDirect(std::vector<std::vector<Bone>>& directKeys)
{
    this->keyBones = directKeys;
}

void SkeletonAni::SetQuaternionsDirect(dx::SimpleMath::Quaternion directQuat)
{
    this->rotQ = directQuat;
}

void SkeletonAni::SetTransVector(dx::SimpleMath::Vector3 transVec)
{
    this->transV = transVec;
}

void SkeletonAni::MergeKeys(std::vector<std::vector<Bone>> keys, float factor, float size)
{

   /* float* kf1 = (float*)(&this->keyBones);
    float* kf2 = (float*)(&keys);

    const size_t elements = sizeof(Bone) / sizeof(float);
    float* result = new float[elements];

    for (size_t i = 0; i < elements; i++)
    {
        result[i] = kf1[i] * (1 - factor) + kf2[i] * factor;
    }

    std::vector<std::vector<Bone>> newBones = *((std::vector<std::vector<Bone>>*)(result));
    delete[] result;

    this->keyBones = newBones;*/

    //F�RS�K 2
   
    //std::cout << "length: " << this->length << std::endl;
    //std::cout << "fps: " << this->fps << std::endl;

    //dx::SimpleMath::Quaternion quatCalc;
    //dx::SimpleMath::Vector3 vecCalc;


    //for (auto k = 0; k < size; k++)
    //{
    //   
    //   std::cout << "size: " << k << std::endl;

    //   float* frame1 = &this->keyBones[k][0].frame;
    //   float* frame2 = &keys[k][0].frame;

    //   unsigned int* index1 = &this->keyBones[k][0].index;
    //   unsigned int* index2 = &keys[k][0].index;

    //   dx::SimpleMath::Quaternion* quat1 = &this->keyBones[k][0].rotationQuaternion;
    //   dx::SimpleMath::Quaternion* quat2 = &keys[k][0].rotationQuaternion;

    //   dx::SimpleMath::Vector3* vec1 = &this->keyBones[k][0].translationVector;
    //   dx::SimpleMath::Vector3* vec2 = &keys[k][0].translationVector;



    //   size_t frames = sizeof(size) / sizeof(float);
    //   size_t indicies = sizeof(size) / sizeof(int);
    //   size_t quaternions = sizeof(size) / sizeof(dx::SimpleMath::Quaternion);
    //   size_t vectors = sizeof(size) / sizeof(dx::SimpleMath::Vector3);



    //   float* frameResult = new float[frames];
    //   int* indexResult = new int[indicies];
    //   dx::SimpleMath::Quaternion* quatResult = new dx::SimpleMath::Quaternion[quaternions];
    //   dx::SimpleMath::Vector3* vectorResult = new dx::SimpleMath::Vector3[vectors];



    //   for (size_t i = 0; i < frames; i++)
    //   {
    //       frameResult[i] = frame1[i] * (1 - factor) + frame2[i] * factor;
    //   }

    //   for (size_t i = 0; i < indicies; i++)
    //   {
    //       indexResult[i] = index1[i] * (1 - factor) + index2[i] * factor;
    //   }

    //   for (size_t i = 0; i < quaternions; i++)
    //   {
    //       quatResult[i] = quatCalc.Slerp(quat1[i], quat2[i], factor);
    //   }

    //   for (size_t i = 0; i < vectors; i++)
    //   {
    //       vectorResult[i] = vecCalc.Lerp(vec1[i], vec2[i], factor);
    //   }


    //   //Bone newBones;
    //   this->keyBones[k][0].frame = frameResult[k];
    //   this->keyBones[k][0].index = indexResult[k];
    //   this->keyBones[k][0].rotationQuaternion = quatResult[k];
    //   this->keyBones[k][0].translationVector = vectorResult[k];

    //   //this->keyBones[k][0] = newBones;

    //   delete[] frameResult;
    //   delete[] indexResult;
    //   delete[] quatResult;
    //   delete[] vectorResult;
    //}

   
    //F�RS�K 3

   /* float frame1;
    float frame2;
    float frameFinal;

    int index1;
    int index2;
    int indexFinal;
        
    dx::SimpleMath::Quaternion quat1;
    dx::SimpleMath::Quaternion quat2;
    dx::SimpleMath::Quaternion quatCalc;
    dx::SimpleMath::Quaternion quatFinal;

    dx::SimpleMath::Vector3 vec1;
    dx::SimpleMath::Vector3 vec2;
    dx::SimpleMath::Vector3 vecCalc;
    dx::SimpleMath::Vector3 vecFinal;

    for (auto i = 0; i < 38; i++)
    {
        std::cout << "size: " << i << std::endl;
        frame1 = this->keyBones[i][0].frame;
        frame2 = keys[i][0].frame;
        frameFinal = frame1 + (frame2 - frame1) * factor;

        index1 = this->keyBones[i][0].index;
        index2 = keys[i][0].index;
        indexFinal = index1 + (index2 - index1) * factor;

        quat1 = this->keyBones[i][0].rotationQuaternion;
        quat2 = keys[i][0].rotationQuaternion;
        quatFinal = quatCalc.Slerp(quat1, quat2, factor);

        vec1 = this->keyBones[i][0].translationVector;
        vec2 = keys[i][0].translationVector;
        vecFinal = vecCalc.Lerp(vec1, vec2, factor);

        this->keyBones[i][0].frame = frameFinal;
        this->keyBones[i][0].index = indexFinal;
        this->keyBones[i][0].rotationQuaternion = quatFinal;
        this->keyBones[i][0].translationVector = vecFinal;

    }
    */
    //delete[] frame1;
    //delete[] frame2;
    ////delete[] frameFinal;
    //delete[] index1;
    //delete[] index2;
    ////delete[] indexFinal;
    //delete[] quat1;
    //delete[] quat2;
    ////delete[] quatFinal;
    //delete[] vec1;
    //delete[] vec2;
    ////delete[] vecFinal;

    

    //M�STE S�TTA ALLA SAKERNA MED LOOPAR - TROLIGTVIS
    //kanske inte ens �r n�ra :(

    
    
}

void SkeletonAni::MergeOffsets(std::vector<dx::SimpleMath::Matrix> offset, float factor, float i)
{
    this->offsetM[i] = this->offsetM[i] * (1 - factor) + offset[i] * factor;
}

void SkeletonAni::SetBlendFPS(float fps, float factor)
{
    //float blendFps1 = this->fps;
    //float blendFps2 = fps;

    //float result;

    //result = blendFps1 * (1 - factor) + blendFps2 * factor;

    //return result;

    this->fps = this->fps * (1 - factor) + fps * factor;
}

void SkeletonAni::SetBlendAnimLength(float animLength, float factor)
{
    //float blendLength1 = this->length;
    //float blendLength2 = animLength;

    //float result;

    //result = blendLength1 * (1 - factor) + blendLength2 * factor;

    //return result;

    this->length = this->length * (1 - factor) + animLength * factor;
}






