program Sample3
	implicit none
	include 'cgnslib_f.h'

	integer:: fin, ier, discharge_size, i, j
	integer:: isize, jsize
	double precision, dimension(:,:), allocatable:: grid_x, grid_y
	double precision, dimension(:,:), allocatable:: elevation
	integer, dimension(:,:), allocatable:: obstacle

	! CGNS �t�@�C���̃I�[�v��
	call cg_open_f('test.cgn', CG_MODE_MODIFY, fin, ier)
	if (ier /=0) STOP "*** Open error of CGNS file ***"

	! iRIClib �̏�����
	call cg_iric_init_f(fin, ier)
	if (ier /=0) STOP "*** Initialize error of CGNS file ***"

	! �i�q�̃T�C�Y�𒲂ׂ�
	call cg_iric_gotogridcoord2d_f(isize, jsize, ier)

	! �i�q��ǂݍ��ނ��߂̃��������m��
	allocate(grid_x(isize,jsize), grid_y(isize,jsize))
	! �i�q��ǂݍ���
	call cg_iric_getgridcoord2d_f(grid_x, grid_y, ier)

	if (ier /=0) STOP "*** No grid data ***"
	! �i�o�́j
	print *, 'grid x,y: isize, jsize=', isize, jsize
	do i = 1, min(isize,5)
		do j = 1, min(jsize,5)
			print *, ' (',i,',',j,')=(',grid_x(i,j),',',grid_y(i,j),')'
		end do
	end do

	! �i�q�_�Œ�`���ꂽ���� elevation �̃��������m��
	allocate(elevation(isize, jsize))
	! ������ǂݍ���
	call cg_iric_read_grid_real_node_f('Elevation', elevation, ier)
	print *, 'Elevation: isize, jsize=', isize, jsize
	do i = 1, min(isize,5)
		do j = 1, min(jsize,5)
			print *, ' (',i,',',j,')=(',elevation(i,j),')'
		end do
	end do

	! �Z���Œ�`���ꂽ���� obstacle �̃��������m�ہB�Z���̑����Ȃ̂ŃT�C�Y�� (isize-1) * (jsize-1)
	allocate(obstacle(isize-1, jsize-1))
	! ������ǂݍ���
	call cg_iric_read_grid_integer_cell_f('Obstacle', obstacle, ier)
	print *, 'Obstacle: isize -1, jsize-1=', isize-1, jsize-1
	do i = 1, min(isize-1,5)
		do j = 1, min(jsize-1,5)
			print *, ' (',i,',',j,')=(',obstacle(i,j),')'
		end do
	end do

	! allocate �Ŋm�ۂ������������J��
	deallocate(grid_x, grid_y, elevation, obstacle)

	! CGNS �t�@�C���̃N���[�Y
	call cg_close_f(fin, ier)
	stop
end program Sample3
