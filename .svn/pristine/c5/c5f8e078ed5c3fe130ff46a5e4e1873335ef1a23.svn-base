      program test_mpi_f

      include 'mpif.h'

      integer error
      integer total_nodes, node
      
      call mpi_init(error)
      call mpi_comm_size(MPI_COMM_WORLD, total_nodes, error)
      call mpi_comm_rank(MPI_COMM_WORLD, node, error)

      write(*, 1000)
 1000 format(//, ' OK ! ',/)



      call mpi_finalize(error)

      end
      
