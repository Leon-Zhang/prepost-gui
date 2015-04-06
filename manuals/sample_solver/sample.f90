program SampleProgram
	implicit none
	include 'cgnslib_f.h'
  integer:: fin, ier
  integer:: icount, istatus
	character(200)::condFile
	integer:: maxiterations
	double precision:: timestep
	integer:: surfacetype
	double precision:: constantsurface
	integer:: variable_surface_size
	double precision, dimension(:), allocatable:: variable_surface_time
	double precision, dimension(:), allocatable:: variable_surface_elevation

	integer:: isize, jsize
	double precision, dimension(:,:), allocatable:: grid_x, grid_y
	double precision, dimension(:,:), allocatable:: elevation
	integer, dimension(:,:), allocatable:: obstacle

	integer:: inflowid
	integer:: inflow_count
	integer:: inflow_element_max
	integer:: discharge_variable_sizemax
	integer, dimension(:), allocatable:: inflow_element_count
	integer, dimension(:,:,:), allocatable:: inflow_element
	integer, dimension(:), allocatable:: discharge_type
	double precision, dimension(:), allocatable:: discharge_constant
	integer, dimension(:), allocatable:: discharge_variable_size
	double precision, dimension(:,:), allocatable:: discharge_variable_time
	double precision, dimension(:,:), allocatable:: discharge_variable_value

	double precision:: time
	integer:: iteration
	double precision, dimension(:,:), allocatable:: velocity_x, velocity_y
	double precision, dimension(:,:), allocatable:: depth
	integer, dimension(:,:), allocatable:: wetflag
	double precision:: convergence

  write(*,*) "Sample Program"

	icount = nargs()
	if ( icount.eq.2 ) then
		call getarg(1, condFile, istatus)
	else
		write(*,*) "Input File not specified."
		stop
  endif

  ! �v�Z�f�[�^�t�@�C�����J��
	call cg_open_f(condFile, CG_MODE_MODIFY, fin, ier)
	if (ier /=0) stop "*** Open error of CGNS file ***"

	! iRIClib �̏�����
	call cg_iric_init_f(fin, ier)
	if (ier /=0) STOP "*** Initialize error of CGNS file ***"

  ! �v�Z�����̓ǂݍ���
	call cg_iric_read_integer_f("maxIteretions", maxiterations, ier)
	call cg_iric_read_real_f("timeStep", timestep, ier)
	call cg_iric_read_integer_f("surfaceType", surfacetype, ier)
	call cg_iric_read_real_f("constantSurface", constantsurface, ier)

	call cg_iric_read_functionalsize_f("variableSurface", variable_surface_size, ier)
	allocate(variable_surface_time(variable_surface_size))
	allocate(variable_surface_elevation(variable_surface_size))
	call cg_iric_read_functional_f("variableSurface", variable_surface_time, variable_surface_elevation, ier)

	! �i�q�̃T�C�Y�𒲂ׂ�
	call cg_iric_gotogridcoord2d_f(isize, jsize, ier)

	! �i�q��ǂݍ��ނ��߂̃��������m��
	allocate(grid_x(isize,jsize), grid_y(isize,jsize))
	! �i�q��ǂݍ���
	call cg_iric_getgridcoord2d_f(grid_x, grid_y, ier)

	! �i�q�_�Œ�`���ꂽ���� �̃��������m��
	allocate(elevation(isize, jsize))
	allocate(obstacle(isize - 1, jsize - 1))

	! ������ǂݍ���
	call cg_iric_read_grid_real_node_f("Elevation", elevation, ier)
	call cg_iric_read_grid_integer_cell_f("Obstacle", obstacle, ier)

	! �������̐���ǂݍ���
	call cg_iric_read_bc_count_f('inflow', inflow_count)

	! �������̐��ɏ]���āA���E������ێ����郁�������m�ہB
	allocate(inflow_element_count(inflow_count))
	allocate(discharge_type(inflow_count), discharge_constant(inflow_count))
	allocate(discharge_variable_size(inflow_count))

	! �������Ɏw�肳�ꂽ�i�q�_�̐��ƁA���Ԉˑ��̗����ʂ̃T�C�Y�𒲂ׂ�
	inflow_element_max = 0
	do inflowid = 1, inflow_count
		! �������Ɏw�肳�ꂽ�i�q�_�̐�
		call cg_iric_read_bc_indicessize_f('inflow', inflowid, inflow_element_count(inflowid))
		if (inflow_element_max < inflow_element_count(inflowid)) then
			inflow_element_max = inflow_element_count(inflowid)
		end if
		! �������̎��Ԉˑ��̗����ʂ̃f�[�^�̐�
		call cg_iric_read_bc_functionalsize_f('inflow', inflowid, 'FunctionalDischarge', discharge_variable_size(inflowid), ier);
		if (discharge_variable_sizemax < discharge_variable_size(inflowid)) then
			discharge_variable_sizemax = discharge_variable_size(inflowid)
		end if
	end do

	! �������Ɏw�肳�ꂽ�i�q�_�ƁA���Ԉˑ��̗����ʂ�ێ����郁�������m�ہB
	allocate(inflow_element(inflow_count, 2, inflow_element_max))
	allocate(discharge_variable_time(inflow_count, discharge_variable_sizemax))
	allocate(discharge_variable_value(inflow_count, discharge_variable_sizemax))

	! ���E�����̓ǂݍ���
	do inflowid = 1, inflow_count
		! �������Ɏw�肳�ꂽ�i�q�_
		call cg_iric_read_bc_indices_f('inflow', inflowid, inflow_element(inflowid:inflowid,:,:), ier)
		! �����ʂ̎�� (0 = ���A1 = ���Ԉˑ�)
		call cg_iric_read_bc_integer_f('inflow', inflowid, 'Type', discharge_type(inflowid:inflowid), ier)
		! ������ (���)
		call cg_iric_read_bc_real_f('inflow', inflowid, 'ConstantDischarge', discharge_constant(inflowid:inflowid), ier)
		! ������ (���Ԉˑ�)
		call cg_iric_read_bc_functional_f('inflow', inflowid, 'FunctionalDischarge', discharge_variable_time(inflowid:inflowid,:), discharge_variable_value(inflowid:inflowid,:), ier)
	end do

	allocate(velocity_x(isize,jsize), velocity_y(isize,jsize), depth(isize,jsize), wetflag(isize,jsize))
	iteration = 0
	time = 0
	do
		time = time + timestep
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
		iteration = iteration + 1
		if (iteration > maxiterations) exit
	end do

	! �v�Z�f�[�^�t�@�C�������
	call cg_close_f(fin, ier)
	stop
end program SampleProgram
