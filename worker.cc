#include <iostream>
#include <zmqpp/zmqpp.hpp>
#include <string>
#include <chrono>
#include <thread>
#include <typeinfo>
#include <math.h> 
#include "json.hpp"
#include "Threadpool.h"

// for convenience
using json = nlohmann::json;

using namespace std;
using namespace zmqpp;

void getCofactor(double** mat, double** temp, int p, int q, int n) 
{ 
    int i = 0, j = 0; 
  
    // Looping for each element of the matrix 
    for (int row = 0; row < n; row++) 
    { 
        for (int col = 0; col < n; col++) 
        { 
            //  Copying into temporary matrix only those element 
            //  which are not in given row and column 
            if (row != p && col != q) 
            { 
                temp[i][j++] = mat[row][col]; 
  
                // Row is filled, so increase row index and 
                // reset col index 
                if (j == n - 1) 
                { 
                    j = 0; 
                    i++; 
                } 
            } 
        } 
    } 
} 
  
/* Recursive function for finding determinant of matrix. 
   n is current dimension of mat[][]. */
double determinantOfMatrix(double** mat, int n) 
{ 
    double D = 0; // Initialize result 
  
    //  Base case : if matrix contains single element 
    if (n == 1) 
        return mat[0][0]; 
  
    double** temp= new double*[n]; // To store cofactors 
    for(int i=0; i<n; i++){
      temp[i]=new double[n];
    }
  
    int sign = 1;  // To store sign multiplier 
  
     // Iterate for each element of first row 
    for (int f = 0; f < n; f++) 
    { 
        // Getting Cofactor of mat[0][f] 
        getCofactor(mat, temp, 0, f, n); 
        D += sign * mat[0][f] * determinantOfMatrix(temp, n - 1); 
  
        // terms are to be added with alternate sign 
        sign = -sign; 
    } 
  
    return D; 
} 

void display(double** mat, int row, int col) 
{ 
    for (int i = 0; i < row; i++) 
    { 
        for (int j = 0; j < col; j++) 
            printf("  %lf", mat[i][j]); 
        printf("\n"); 
    } 
} 

double* resta(double* a, double* b,int dimension){
  double* v=new double[dimension];
  for(int i=0; i<dimension;i++){
    v[i]=a[i]-b[i];
  }
  return v;
}

double* multiply(double* a, double n, int dimension){
  double* v=new double[dimension];
  for(int i=0;i<dimension;i++){
    v[i]=n*a[i];
  }
  return v;
}

double multiplyv(double* col, double* row, int dimension){
  double v=0;
  for(int i=0; i<dimension; i++){
    v=v+col[i]*row[i];
  }
  return v;
}

void diplayv(double* n, int dimension){
  for(int i=0; i<dimension;i++)
    cout<<n[i]<<" ";
  cout<<endl;
}

double N(double* data, double* mean, double** cov, int dimension){
  double det= determinantOfMatrix(cov,dimension);
  double* r=resta(data,mean,dimension);
  double* a=multiply(r,-0.5,dimension);
  double* b=multiply(r,1/det,dimension);
  double c=multiplyv(a,b,dimension);
  double f=exp(c);

  double d=pow(2*M_PI,dimension/2);
  double e=d*pow(abs(det),0.5);

  cout<<det<<" "<<c<<" "<<f<<endl;
  return f/e;
}


double** means(json j){
  int d=j["dimension"].get<int>();
  int c=j["clusters"].get<int>();
  double** v= new double*[c];
  for(int i=0; i<c; i++){
    v[i]=new double[d];
  }
  for(int i=0; i<c; i++)
    for(int t=0; t<d; t++)
      v[i][t]=j["means"][i][t].get<double>();
  return v;
}

double* weights(json j){
  int c=j["clusters"].get<int>();
  double* v= new double[c];
  for(int i=0; i<c; i++)
    v[i]=j["weights"][i].get<double>();
  return v;
}

double*** covs(json j){
  int d=j["dimension"].get<int>();
  int c=j["clusters"].get<int>();
  double*** v= new double**[c];
  for(int i=0; i<c; i++){
    v[i]=new double*[d];
    for(int t=0; t<d; t++){
      v[i][t]=new double[d];
    }
  }
  for(int i=0; i<c; i++)
    for(int t=0; t<d; t++)
      for(int s=0; s<d; s++)
        v[i][t][s]=j["covs"][i][t][s].get<double>();
  return v;
}

double** data(json j){
  int d=j["dimension"].get<int>();
  int s=j["size"].get<int>();
  double** v= new double*[s];
  for(int i=0; i<s; i++){
    v[i]=new double[d];
  }
  for(int i=0; i<s; i++)
    for(int t=0; t<d; t++)
      v[i][t]=j["data"][i][t].get<double>();
  return v;
}


double suma(double* r, int clusters){
  double v=0;
  for(int i=0; i<clusters; i++)
    v=v+r[i];
  return v;
}

string covstoString(double*** v, int clusters, int dimension){
  string r="[";
  for(int n=0; n<clusters;n++){
    r=r+"[";
    for(int i=0;i<dimension;i++){
      r=r+"[";
      for(int j=0; j<dimension; j++){
        r=r+to_string(v[n][i][j]);
        if(j!=dimension-1){
          r=r+",";
        }
      }
      r=r+"]";
      if(i!=dimension-1){
        r=r+",";
      }
    }
    r=r+"]";
    if(n!=clusters-1){
      r=r+",";
    }
  }
  return r+"]";
}

string meanstoString(double** v, int clusters, int dimension){
  string r="[";
  for(int n=0; n<clusters;n++){
    r=r+"[";
    for(int i=0;i<dimension;i++){
      r=r+to_string(v[n][i]);
      if(i!=dimension-1){
        r=r+",";
      }
    }
    r=r+"]";
    if(n!=clusters-1){
      r=r+",";
    }
  }
  return r+"]";
}

string weigthstoString(double* v, int clusters){
  string r="[";
  for(int n=0; n<clusters;n++){
    r=r+to_string(v[n]);
    if(n!=clusters-1){
      r=r+",";
    }
  }
  return r+"]";
}

string estep(json j){
  double** datav=data(j);
  double** meansv=means(j);
  double*** covsv=covs(j);
  double* w=weights(j);
  int d=j["dimension"].get<int>();
  int c=j["clusters"].get<int>();
  int s=j["size"].get<int>();

  double** v=new double*[s];
  for(int i=0; i<s; i++){
    v[i]=new double[c];
  }
  ThreadPool pool(thread::hardware_concurrency());
  vector<future<int>> results;
  for(int i=0; i<s; i++){
    results.emplace_back(
      pool.enqueue([datav,meansv,i,c,covsv,d,v,w] {
      for(int z=0; z<c; z++){
        v[i][z]=w[z]*N(datav[i],meansv[z],covsv[z],d);
      }
      return 1;
      })
    );
  }
  for(auto && result: results){
    int resultados = result.get();
  }
  //display(v,s,c);
  
  for(int i=0; i<s; i++){
    double b=suma(v[i],c);
    for(int z=0; z<c;z++){
      v[i][z]=v[i][z]/b;
    }
  }

  string j2="{'means':";
  j2=j2+meanstoString(meansv,c,d)+",";
  j2=j2+"'responsability':";
  j2=j2+meanstoString(v,s,c)+",";
  j2=j2+"'weigths':";
  j2=j2+weigthstoString(w,c)+",";
  j2=j2+"'data':";
  j2=j2+meanstoString(datav,s,d)+",";
  j2=j2+"'clusters':";
  j2=j2+to_string(c)+",";
  j2=j2+"'covs':";
  j2=j2+covstoString(covsv,c,d)+"}";
  return j2;
}

int main() {
  
  context ctx;
  socket work(ctx, socket_type::pull);
  work.connect("tcp://localhost:5557");
  
  socket sink(ctx, socket_type::push);
  sink.connect("tcp://localhost:5559");
  message rep;
  rep << "hi!";
  sink.send(rep);
  

  while(true) {
    
    message s;
    work.receive(s);

    string text;
    s >> text;
    
    //string text="{\"means\":[[1.1,2.1],[1.1,2.5]], \"weights\":[0.5,0.5], \"covs\":[[[1,2],[2,1]],[[4,5],[5,4]]], \"data\":[[4,1],[5,4]], \"dimension\":2, \"clusters\":2, \"size\":2}";
    json j=json::parse(text);

    cout << "Work received " <<endl;
    string v=estep(j);

    message rep;
    rep << v;
    sink.send(rep);
  }

  cout << "Hello from C++ folks" << endl;
  return 0;
}
