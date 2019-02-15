#ifndef WEIGHTS_H
#define WEIGHTS_H

#include <vector>

class Weights
{
public:
    Weights();
    Weights(int vertexNumber, int handleNumber);

    void create(int vertexNumber, int handleNumber);
    void clear();

    void setWeight(const int handleId, const int vertexId, const double weight);
    double getWeight(const int handleId, const int vertexId) const;
    const std::vector<double> & getWeights(const int vertexId) const;

    double getWeightsSum(const int vertexId) const;
    void forcePartitionOfUnity();

    inline int getNumberOfVertices() const { return vertexNumber; }
    inline int getNumberOfHandles()  const { return handleNumber; }


private:
    std::vector<std::vector<double>> weights;
    int vertexNumber;
    int handleNumber;
};

#endif // WEIGHTS_H
