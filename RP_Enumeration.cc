#include "RP_Enumeration.hh"

void EnumerationRPOpt::First()
{ // insert first router

    point router;
    router.row = 0;
    router.col = 0;

    while (in.IsWall(router.row, router.col))
    {
        if (router.col < in.Columns()-1)
            router.col++;
        else if (router.row < in.Rows()-1 && router.col == in.Columns()-1) 
            router.row++;
    }

    out.InsertRouter(router.row, router.col);
    out.ConnectRouterWithBackbone(router);
}


bool EnumerationRPOpt::Next()
{
    point router;
    unsigned k = 0;

#if DEBUG
    cout << "Next: " << endl;
    cout << out << endl << out.RemainingBudget() << endl;
#endif

    for (unsigned r = 0; r < in.Rows(); r++)
    {
        for (unsigned c = 0; c < in.Columns(); c++)
        {
            if (out.Cells()[r][c] == 'r')
            {
                router.col = c;
                router.row = r;
            
                if (out.Cell(out.NextCell(r,c).row, out.NextCell(r,c).col) == 'r')
                {
                    if (in.IsGridPoint(out.NextCell(r,c).row, out.NextCell(r,c).col))
                        k++;  
                }
                else if (in.IsGridPoint(out.NextCell(r,c).row, out.NextCell(r,c).col))
                {   
                    out.RemoveRouter(router);

                    out.InsertRouter(out.NextCell(r,c).row, out.NextCell(r,c).col);

                    int count;
                    point temp;
                    temp.row = r;
                    temp.col = c;

                    for (unsigned m = 0; m < k; m++)
                    {
                        count = m - k;
                        for (int i = 0; i < abs(count); i++)
                        {
                            if (in.IsGridPoint(out.PreviousCell(temp.row, temp.col).row, out.PreviousCell(temp.row, temp.col).col))
                            {
                                out.RemoveRouter(temp = out.PreviousCell(temp.row, temp.col));
                            }
                        }
                    }


                    count = 0;
                    for (unsigned r1 = 0; r1 < in.Rows(); r1++)
                    {
                        for (unsigned c1 = 0; c1 < in.Columns(); c1++)
                        {
                            if (count == k)
                                break;
                            out.InsertRouter(r1,c1);
                            count++;
                        }
                    }

                    out.UpdateCoverage();
                    out.UpdateBackbone();
                    return true;  
                }
            }
        }
    }

    // the only situation in which next is false: all cells are routers
    if (out.RoutersSize() == in.Rows()*in.Columns())
        return false;

    unsigned routers = out.RoutersSize();

    out.RemoveAllRoutersAndRestartBackbone();

    // once the board was cleared, start counting with 1 more router 
    unsigned n = 0;
    for (unsigned r = 0; r < in.Rows(); r++)
    {
        for (unsigned c = 0; c < in.Columns(); c++)
        {
            if (n == routers+1)
                break;
            out.InsertRouter(r,c);
            n++;
        }
    }

    // update data
    out.UpdateCoverage();
    out.UpdateBackbone(); 
    return true;
}


bool EnumerationRPOpt::Feasible()
{
    // check budget constraint
    if (out.RemainingBudget() < 0)
        return false;

    // check routers constraint
    for (unsigned i = 0; i < out.RoutersSize(); i++)
        if (in.IsWall(out.Router(i).row, out.Router(i).col))
            return false;

    return true;
}


int EnumerationRPOpt::Cost()
{
    // define cost as the opposite of score
    int score = 1000*out.TotalCoveredPoints()+in.Budget()-out.RoutersSize()*in.RouterPrice()-out.BackboneSize()*in.BackbonePrice();
    return -score;
}