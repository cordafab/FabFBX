#include "weights.h"

Weights::Weights()
{
    create(0,0);
}

Weights::Weights(int vertexNumber, int handleNumber)
{
   create(vertexNumber, handleNumber);
}

void Weights::create(int vertexNumber, int handleNumber)
{
   weights.resize(vertexNumber);
   this->vertexNumber = vertexNumber;
   this->handleNumber = handleNumber;
   for(int i=0; i<vertexNumber; ++i)
       weights[i].resize(handleNumber,0);
}

void Weights::clear()
{
    weights.clear();
    vertexNumber = 0;
    handleNumber = 0;
}

void Weights::setWeight(const int handleId, const int vertexId, const double weight)
{
   if((unsigned long)vertexId < weights.size() && (unsigned long)handleId < weights[vertexId].size())
      weights[vertexId][handleId] = weight;
}

double Weights::getWeight(const int handleId, const int vertexId) const
{
   try
   {
      double weight = weights[vertexId][handleId];
      return weight;
   }
   catch (const std::out_of_range & e)
   {
      return 0;
   }
}

const std::vector<double> & Weights::getWeights(const int vertexId) const
{
   //TODO: assert if vertexId out_of_bound
   return weights[vertexId];
}

double Weights::getWeightsSum(const int vertexId) const
{
   double res = 0.0;
   std::vector<double> weights = getWeights(vertexId);

   for(int i=0; i<handleNumber; ++i)
   {
      res += weights[i];
   }

   return res;
}

void Weights::forcePartitionOfUnity()
{
   /*for(int i = 0; i<vertexNumber; ++i)
   {
      double weightsSum = getWeightsSum(i);
      if(weightsSum < 1)
      {
         double difference = 1-weightsSum;
         int max = 0;
         for(int j=0; j<handleNumber; ++j)
         {
            if(weights[i][max]<weights[i][j]) max = j;
         }
         weights[i][max]+=difference;
      }

   }*/
}
