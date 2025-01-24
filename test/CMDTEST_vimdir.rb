require 'etc'

class CMDTEST_basic < Cmdtest::Testcase
  def test_help
    cmd "vimdir -h" do
      exit_zero
      stdout_equal /.+/
    end
  end

  def test_no_color
    cmd "NO_COLOR=1 vimdir -h" do
      exit_zero
      stdout_equal /^[^\e]+$/m
    end
  end

  def test_wrong_arg
    cmd "vimdir -j" do
      exit_nonzero
      stdout_equal /.+/
      stderr_equal /.+/
    end
  end

  def test_missing_folder
    cmd "vimdir ./this/directory/does/not/exist/" do
      exit_nonzero
      created_files ["vimdir_test_file.vimdir"]
      stderr_equal /.+error.+/
    end
  end

  def test_editor
    import_file "test/saver.sh", "./"

    cmd "EDITOR=./saver.sh vimdir ./" do
      exit_zero
      created_files ["vimdir_test_file.vimdir", "output.txt"]
    end
  end

  def test_vimdir_editor
    import_file "test/saver.sh", "./"

    cmd "VIMDIREDITOR=./saver.sh vimdir ./" do
      exit_zero
      created_files ["vimdir_test_file.vimdir", "output.txt"]
    end
  end
end

class CMDTEST_mydir < Cmdtest::Testcase
  def setup
    import_file "test/replacer.sh",   "./"
    import_file "test/saver.sh",      "./"
    import_file "test/memoryhole.sh", "./"
    import_directory "test/mydir/", "./mydir/"
  end

  def test_noop
    cmd "EDITOR=./memoryhole.sh vimdir -n ./mydir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir"]
    end
  end

  def test_contents
    expected = [
        "000\t./mydir/.gitkeep",
        "001\t./mydir/file.txt",
        "002\t./mydir/script.sh",
    ]

    cmd "EDITOR=./saver.sh vimdir -n ./mydir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir", "output.txt"]
      file_equal "vimdir_test_file.vimdir", expected
    end
  end

  def test_permission_contents
    expected = [
        "000\t-rw-r--r--\t./mydir/.gitkeep",
        "001\t-rw-r--r--\t./mydir/file.txt",
        "002\t-rwxr-xr-x\t./mydir/script.sh",
    ]

    cmd "EDITOR=./saver.sh vimdir -n -p ./mydir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir", "output.txt"]
      file_equal "vimdir_test_file.vimdir", expected
    end
  end

  def test_owner_contents
    username  = Etc.getpwuid(1000).name
    groupname = Etc.getgrgid(1000).name
    expected = [
        "000\t#{username}:#{groupname}\t./mydir/.gitkeep",
        "001\t#{username}:#{groupname}\t./mydir/file.txt",
        "002\t#{username}:#{groupname}\t./mydir/script.sh",
    ]

    cmd "EDITOR=./saver.sh vimdir -n -o ./mydir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir", "output.txt"]
      file_equal "vimdir_test_file.vimdir", expected
    end
  end

  def test_permissoin_owner_contents
    username  = Etc.getpwuid(1000).name
    groupname = Etc.getgrgid(1000).name
    expected = [
        "000\t-rw-r--r--\t#{username}:#{groupname}\t./mydir/.gitkeep",
        "001\t-rw-r--r--\t#{username}:#{groupname}\t./mydir/file.txt",
        "002\t-rwxr-xr-x\t#{username}:#{groupname}\t./mydir/script.sh",
    ]

    cmd "EDITOR=./saver.sh vimdir -n -p -o ./mydir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir", "output.txt"]
      file_equal "vimdir_test_file.vimdir", expected
    end
  end

  def test_del_file
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "002\t./mydir/script.sh",
        "001\t./mydir/file.txt",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n ./mydir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir"]
      removed_files ["target.txt"]
    end
  end

  def test_swapped_order_noop
    File.write('target.txt',
      [
        "000\t./mydir/.gitkeep",
        "002\t./mydir/script.sh"
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n ./mydir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir"]
      removed_files ["target.txt"]
      stderr_equal /^.*delete '.*file.txt'.*$/
    end
  end

  def test_chmod_file
    File.write('target.txt',
      [
        "000\t-rw-r--r--\t./mydir/.gitkeep",
        "001\t-rw-r--r--\t./mydir/file.txt",
        "002\t-rw-r--r--\t./mydir/script.sh",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n -p ./mydir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir"]
      removed_files ["target.txt"]
      stderr_equal /^.*chmod '.*script.sh' (.+).*$/
    end
  end
end

class CMDTEST_mynesteddir < Cmdtest::Testcase
  def setup
    import_file "test/replacer.sh",   "./"
    import_file "test/saver.sh",      "./"
    import_file "test/memoryhole.sh", "./"
    import_directory "test/mynesteddir/", "./mynesteddir/"
  end

  def test_trailing_slash_included
    cmd "EDITOR=./saver.sh vimdir -n ./mynesteddir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir", "output.txt"]
      file_equal "vimdir_test_file.vimdir", /^000\t.+nest\/$/
    end
  end

  def test_recursive
    expected = [
        "000\t./mynesteddir/nest/",
        "001\t./mynesteddir/nest/.gitkeep",
    ]

    cmd "EDITOR=./saver.sh vimdir -n -r ./mynesteddir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir", "output.txt"]
      file_equal "vimdir_test_file.vimdir", expected
    end
  end
end

class CMDTEST_myswapdir < Cmdtest::Testcase
  def setup
    import_file "test/replacer.sh", "./"
    import_directory "test/myswapdir/", "./myswapdir/"
  end

  def test_swap
    File.write('target.txt',
      [
        "000\t./myswapdir/file2.txt",
        "002\t./myswapdir/file1.txt",
      ].join("\n")
    )

    cmd "EDITOR=./replacer.sh vimdir -n ./myswapdir/" do
      exit_zero
      created_files ["vimdir_test_file.vimdir"]
      removed_files ["target.txt"]
      stderr_equal /.+/
    end
  end
end
