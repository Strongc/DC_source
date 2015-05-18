#include <assert.h>

#define NO_OF_REF_POINTS 10

int mNumberOfReferencePoints;
float mSortedLevels[NO_OF_REF_POINTS];
float mFlowsOfLevels[NO_OF_REF_POINTS];

void Init()
{
  //clear arrays
  for (int i = 0; i < NO_OF_REF_POINTS; i++)
  {
    mSortedLevels[i] = 0.0f;
    mFlowsOfLevels[i] = 0.0f;
  }
  mNumberOfReferencePoints = 0;
}


void Sort()
{
  bool sorting_completed = false;
  
  // sort the reference points by level
  // use simple bubble sort because number of elements is low
  while (!sorting_completed)
  {
    sorting_completed = true;

    for (int i = 0; i < mNumberOfReferencePoints - 1; i++)
    {
      if (mSortedLevels[i] > mSortedLevels[i + 1])
      {
        float level = mSortedLevels[i];
        float flow = mFlowsOfLevels[i];

        mSortedLevels[i] = mSortedLevels[i + 1];
        mFlowsOfLevels[i] = mFlowsOfLevels[i + 1];
        
        mSortedLevels[i + 1] = level;
        mFlowsOfLevels[i + 1] = flow;

        sorting_completed = false;
      }
    }
  }
  
}




/*****************************************************************************
 * Function - GetFlowAtLevel
 * DESCRIPTION: Calculate flow from array of reference points sorted with ascending levels
 *
 *****************************************************************************/
float GetFlowAtLevel(float level)
{
  float flow = 0.0f;

  bool flow_found = false;

  if (mNumberOfReferencePoints == 1)
  {
    flow =  mFlowsOfLevels[0];
  }
  else if (mNumberOfReferencePoints > 1)
  {
    // find the two adherent reference points to use (or single point on exact match)
    for (int i = 0; i < mNumberOfReferencePoints - 1; i++)
    {
      if (level == mSortedLevels[i])
      {
        flow = mFlowsOfLevels[i];
        flow_found = true;
        break;
      }

      // liniear coeff between current and next point
      float coeff = (mFlowsOfLevels[i + 1] - mFlowsOfLevels[i]) / (mSortedLevels[i + 1] - mSortedLevels[i]); 

      if (level < mSortedLevels[i])
      {
        flow =  mFlowsOfLevels[i] - (mSortedLevels[i] - level) * coeff;
        flow_found = true;
        break;
      }
      else if (mSortedLevels[i] < level && level < mSortedLevels[i + 1])
      {
        flow = mFlowsOfLevels[i] + (level - mSortedLevels[i]) * coeff;
        flow_found = true;
        break;
      }
      //else the level is higher than next point

    }
  }

  int last_point = mNumberOfReferencePoints - 1;

  if (!flow_found && last_point > 0)
  {
    if (level == mSortedLevels[last_point])
    {
      flow = mFlowsOfLevels[last_point];
    }
    else //if level > mSortedLevels[last_point]
    {
      // liniear coeff between last two points
      float coeff = (mFlowsOfLevels[last_point] - mFlowsOfLevels[last_point - 1]) / (mSortedLevels[last_point] - mSortedLevels[last_point - 1]); 

      flow = mFlowsOfLevels[last_point] + (level - mSortedLevels[last_point]) * coeff;
    }
  }

  return flow;
}





void Test1()
{
  Init();

  mSortedLevels[0] = 10;
  mFlowsOfLevels[0] = 10;
  mSortedLevels[1] = 1;
  mFlowsOfLevels[1] = 1;
  mNumberOfReferencePoints = 2;

  Sort();

  float result = GetFlowAtLevel(5);
  assert(result == 5.0f);
  result = GetFlowAtLevel(15);
  assert(result == 15.0f);
}

void Test2()
{
  Init();

  mSortedLevels[0] = 10;
  mFlowsOfLevels[0] = 10;
  mSortedLevels[1] = 1;
  mFlowsOfLevels[1] = 1;
  mSortedLevels[2] = 20;
  mFlowsOfLevels[2] = 15;
  mNumberOfReferencePoints = 3;

  Sort();

  float result = GetFlowAtLevel(5);
  assert(result == 5.0f);
  result = GetFlowAtLevel(15);
  assert(result == 12.5f);
}

void Test3()
{
  Init();

  mSortedLevels[0] = 2;
  mFlowsOfLevels[0] = 5;
  mSortedLevels[1] = 1;
  mFlowsOfLevels[1] = 5;
  mNumberOfReferencePoints = 2;

  Sort();

  float result = GetFlowAtLevel(5);
  assert(result == 5.0f);
  result = GetFlowAtLevel(15);
  assert(result == 5.0f);
}

void Test4()
{
  Init();

  mSortedLevels[0] = 3;
  mFlowsOfLevels[0] = 5;
  mNumberOfReferencePoints = 1;

  Sort();

  float result = GetFlowAtLevel(2);
  assert(result == 5.0f);
}

void Test5()
{
  Init();

  mSortedLevels[0] = 4;
  mFlowsOfLevels[0] = 8;
  mSortedLevels[1] = 8;
  mFlowsOfLevels[1] = 10;
  mSortedLevels[2] = 12;
  mFlowsOfLevels[2] = 15;
  mNumberOfReferencePoints = 3;

  Sort();

  float result = GetFlowAtLevel(0);
  assert(result == 6.0f);
  result = GetFlowAtLevel(2);
  assert(result == 7.0f);
  result = GetFlowAtLevel(4);
  assert(result == 8.0f);
  result = GetFlowAtLevel(8);
  assert(result == 10.0f);
  result = GetFlowAtLevel(9);
  assert(result == 11.25f);
  result = GetFlowAtLevel(14);
  assert(result == 17.5f);
  result = GetFlowAtLevel(16);
  assert(result == 20.0f);
}

int main (void)
{
  Test1();
  Test2();
  Test3();
  Test4();
  Test5();
}
