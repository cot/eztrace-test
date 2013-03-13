      program squelette_RECOM
      implicit none
      integer n, i, j, m
      parameter (n=8, m=100)
      real*8 a(0:n),b(0:n),c(0:n),r

      do i=0,n
         a(i)=sqrt(REAL(i))
         b(i)=0
      enddo

!$OMP PARALLEL

!$OMP DO private(i)
      do i=0,n
         A(i) = A(i) +1
      enddo
!$omp end do nowait


!$OMP DO private(i)
      do i=0,n
         do j=0,((i/10000)+1)*m
            b(i) = b(i) + sqrt(a(i))/3
         enddo
      enddo
!$omp end do nowait

!$OMP DO private(i)
      do i=0,n
         do j=0,((i/10000)+1)*m
            c(i) = c(i) + sqrt(a(i))/3
         enddo
      enddo
!$omp end do

!$OMP DO private(i)
      do i=0,n-1
         do j=0,((i/10000)+1)*m
            b(i+1) = b(i+1) + sqrt(a(i))
         enddo
      enddo
!$omp end do nowait

!$omp do private(i)
      do i=0,n-1
         c(i+1) = c(i+1) + sqrt(a(i))
      enddo
!$omp end do

!$omp do private(i)
      do i=0,n-1
         b(i+1) = b(i+1) + sqrt(a(i)) + c(i)
      enddo
!$omp end do

!$omp master
      do i=0,n
         r=r+b(i)
      enddo

      print *, r
!$omp end master
!$OMP END PARALLEL
      end
