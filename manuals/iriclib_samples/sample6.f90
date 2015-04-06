
program Sample6
	implicit none
	include 'cgnslib_f.h'

	integer:: fin, ier, isize, jsize
	double precision:: time
	double precision:: convergence
	double precision, dimension(:,:), allocatable::grid_x, grid_y
	real, dimension(:,:), allocatable:: velocity_x, velocity_y, depth
	integer, dimension(:,:), allocatable:: wetflag

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
	! �v�Z���ʂ�ێ����郁�������m��
	allocate(velocity_x(isize,jsize), velocity_y(isize,jsize), depth(isize, jsize), wetflag(isize,jsize))
	! �i�q��ǂݍ���
	call cg_iric_getgridcoord2d_f (grid_x, grid_y, ier)

	! ������Ԃ̏����o��
	time = 0
	convergence = 0.1
	call cg_iric_write_sol_time_f(time, ier)
	! �i�q���o��
	call cg_iric_write_sol_gridcoord2d_f (grid_x, grid_y, ier)
	! �v�Z���ʂ��o��
	call cg_iric_write_sol_real_f ('VelocityX', velocity_x, ier)
	call cg_iric_write_sol_real_f ('VelocityY', velocity_y, ier)
	call cg_iric_write_sol_real_f ('Depth', depth, ier)
	call cg_iric_write_sol_integer_f ('Wet', wetflag, ier)
	call cg_iric_write_sol_baseiterative_real_f ('Convergence', convergence, ier)
	do
		time = time + 10.0
		! (�����Ōv�Z�����s�B�i�q�̌`����ω�)
		call cg_iric_write_sol_time_f(time, ier)
		! �i�q���o��
		call cg_iric_write_sol_gridcoord2d_f (grid_x, grid_y, ier)
		! �v�Z���ʂ��o��
		call cg_iric_write_sol_real_f ('VelocityX', velocity_x, ier)
		call cg_iric_write_sol_real_f ('VelocityY', velocity_y, ier)
		call cg_iric_write_sol_real_f ('Depth', depth, ier)
		call cg_iric_write_sol_integer_f ('Wet', wetflag, ier)
		call cg_iric_write_sol_baseiterative_real_f ('Convergence', convergence, ier)

		If (time > 1000) exit
	end do

	! CGNS �t�@�C���̃N���[�Y
	call cg_close_f(fin, ier)
	stop
end program Sample6
