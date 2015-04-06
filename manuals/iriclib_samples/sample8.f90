program Sample8
	implicit none
	include 'cgnslib_f.h'

	integer:: fin, ier, isize, jsize, ksize, i, j, k, aret
	integer:: condid, indexid
	integer:: condcount, indexlenmax, funcsizemax
	integer:: tmplen
	integer, dimension(:), allocatable:: condindexlen
	integer, dimension(:,:,:), allocatable:: condindices
	integer, dimension(:), allocatable:: intparam
	double precision, dimension(:), allocatable:: realparam
	character(len=200), dimension(:), allocatable:: stringparam
	character(len=200):: tmpstr
	integer, dimension(:), allocatable:: func_size
	double precision, dimension(:,:), allocatable:: func_param;
	double precision, dimension(:,:), allocatable:: func_value;

	! CGNS �t�@�C���̃I�[�v��
	call cg_open_f('bctest.cgn', CG_MODE_MODIFY, fin, ier)
	if (ier /=0) STOP "*** Open error of CGNS file ***"

	! iRIClib �̏�����
	call cg_iric_init_f(fin, ier)
	if (ier /=0) STOP "*** Initialize error of CGNS file ***"

	! �������̐����擾����
	call cg_iric_read_bc_count_f('inflow', condcount)
	! �������̐��ɏ]���āA�p�����[�^�̕ۑ��p�̃��������m�ۂ���B
	allocate(condindexlen(condcount), intparam(condcount), realparam(condcount))
	allocate(stringparam(condcount), func_size(condcount))
	print *, 'condcount ', condcount

	! ���E�������ݒ肳�ꂽ�i�q�_�̐��ƁA�֐��^�̋��E�����̍ő�T�C�Y�𒲂ׂ�
	indexlenmax = 0
	funcsizemax = 0
	do condid = 1, condcount
		call cg_iric_read_bc_indicessize_f('inflow', condid, condindexlen(condid), ier)
		if (indexlenmax < condindexlen(condid)) then
			indexlenmax = condindexlen(condid)
		end if
		call cg_iric_read_bc_functionalsize_f('inflow', condid, 'funcparam', func_size(condid), ier);
		if (funcsizemax < func_size(condid)) then
			funcsizemax = func_size(condid)
		end if
	end do
	
	! �i�q�_�̃C���f�b�N�X�i�[�p�̔z��ƁA�֐��^���E�����̊i�[�p�ϐ��̃��������m��
	allocate(condindices(condcount, 2, indexlenmax))
	allocate(func_param(condcount, funcsizemax), func_value(condcount, funcsizemax))
	! �C���f�b�N�X�ƁA���E���� ��ǂݍ���
	do condid = 1, condcount
		call cg_iric_read_bc_indices_f('inflow', condid, condindices(condid:condid,:,:), ier)
		call cg_iric_read_bc_integer_f('inflow', condid, 'intparam', intparam(condid:condid), ier)
		call cg_iric_read_bc_real_f('inflow', condid, 'realparam', realparam(condid:condid), ier)
		call cg_iric_read_bc_string_f('inflow', condid, 'stringparam', tmpstr, ier)
		stringparam(condid) = tmpstr
		call cg_iric_read_bc_functional_f('inflow', condid, 'funcparam', func_param(condid:condid,:), func_value(condid:condid,:), ier)
	end do

	do condid = 1, condcount
		do indexid = 1, condindexlen(condid)
			print *, 'condindices ', condindices(condid:condid,:,indexid:indexid)
		end do
		print *, 'intparam ', intparam(condid:condid)
		print *, 'realparam ', realparam(condid:condid)
		print *, 'stringparam ', stringparam(condid)
		print *, 'funcparam X ', func_param(condid:condid, 1:func_size(condid))
		print *, 'funcparam Y ', func_value(condid:condid, 1:func_size(condid))
	end do
	
	! CGNS �t�@�C���̃N���[�Y
	call cg_close_f(fin, ier)
	stop
end program Sample8
