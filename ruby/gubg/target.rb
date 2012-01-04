require("gubg/graph")

module Target
    attr_reader(:generationState)
    def defineDependencies(klassPerName = {})
        @klassPerName = klassPerName
        @namePerKlass = klassPerName.invert
        raise("There are duplicates in the dependency list for #{self.class}: #{klassPerName}") if @klassPerName.length != @namePerKlass.length
    end
    def dependsOn?(otherTarget)
        raise("Did you forget to define the dependencies for class #{self.class}? Define them using defineDependencies, even if they are empty.") if @namePerKlass.nil?
        @namePerKlass.has_key?(otherTarget.class)
    end
    def setDependencies(targets)
        @targetPerName = {}
        @namePerKlass.each do |klass, name|
            target = targets.find{|target|klass === target}
            raise("Could not find the target of class #{klass}") if target.nil?
            @targetPerName[name] = target
        end
        raise("Some required targets are missing for #{self.class}") if @targetPerName.length != @klassPerName.length
    end
    def getTarget(name)
        @targetPerName[name]
    end
    def getTargets(*names)
        case names.length
        when 0 then return *@targetPerName.values
        when 1 then return @targetPerName[names[0]]
        else
            return *names.map{|n|@targetPerName[n]}
        end
    end

    STATES = [:waitingForDependencies, :firstTime, :generating, :halted, :generated, :error]
    def setGenerationState(generationState)
        raise("I cannot accept generationState #{generationState}, it is not part of #{STATES}") if !STATES.include?(generationState)
        @generationState = generationState
    end
    def generationState?(wantedState)
        @generationState == wantedState
    end
    #You have to provide generate_ yourself. We cannot provide a dummy implementation because including Target will override
    #any default implementation that is provided via inheritance
    def generate
        setGenerationState(:firstTime) if generationState?(:waitingForDependencies)
        setGenerationState(generate_)
    end
    #Provide a progressible? when you have cyclic dependencies that can gradually be solved
    def print
        puts("Target #{self.class}")
    end
end

class TargetGraph < Graph
    def initialize(targets)
        super()
        #Add the edges that define the target graph. An edge points to a dependency.
        targets.each do |target|
            targets.each do |depTarget|
                addEdge(target, depTarget) if target.dependsOn?(depTarget)
            end
            target.setDependencies(outVertices(target))
            target.setGenerationState(:waitingForDependencies)
        end
    end
    def process
        #Collect all unfinished targets
        ungeneratedTargets = vertices.reject{|target|target.generationState?(:generated)}
        return :finished if ungeneratedTargets.empty?

        #Collect all targets that can be generated directly, i.e., all dependent targets are generated
        executableTargets = ungeneratedTargets.select{|target|outVertices(target).all?{|depTarget|depTarget.generationState?(:generated)}}
        if !executableTargets.empty?
            puts("\tThe following targets are directly generatable #{executableTargets.map{|target|target.class}}")
            executableTargets.each do |target|
                target.generate
                target.print
            end
            failedTargets = executableTargets.select{|target|target.generationState != :generated}
            raise("I expected all executable targets to be generated by now, but these are not: #{failedTargets.map{|target|target.class}}") if !failedTargets.empty?
            return :direct
        end

        #Collect all progressible targets
        progressibleTargets = ungeneratedTargets.select{|target|target.progressible?}
        if !progressibleTargets.empty?
            if progressibleTargets.all?{|target|target.generationState?(:halted)}
                #All targets are halted, we give them a chance to become generated, else we stop
                progressibleTargets.each do |target|
                    target.generate
                    target.print
                end
                return :halted if progressibleTargets.all?{|target|target.generationState?(:halted)}
            elsif progressibleTargets.any?{|target|target.generationState?(:error)}
                progressibleTargets.each do |target|
                    target.print if target.generationState?(:error)
                end
                return :error
            else
                puts("\tThe following targets are implicitly generatable #{progressibleTargets.map{|target|target.class}}")
                progressibleTargets.each do |target|
                    target.generate
                    target.print
                end
            end
            return :implicit
        end

        puts("No more progress can be made, everything is blocked")
        return :blocked
    end
end

if __FILE__ == $0
    class A
        include Target
        def initialize
            defineDependencies()
        end
        def generate_
            :generated
        end
    end
    class B
        include Target
        def initialize
            defineDependencies(a: A)
        end
        def generate_
            :generated
        end
    end
    class C
        include Target
        def initialize
            defineDependencies(b: B, d: D)
        end
        def generate_
            getTarget(:d).generationState?(:generated) ? :generated : :halted
        end
        def progressible?
            getTarget(:b).generationState?(:generated)
        end
    end
    class D
        include Target
        def initialize
            defineDependencies(b: B, c: C)
        end
        def generate_
            :generated
        end
        def progressible?
            getTarget(:b).generationState?(:generated)
        end
    end

    targets = [A, B, C, D].map{|klass|klass.new}
    targetGraph = TargetGraph.new(targets)

    cycleCount = -1
    loop do
        cycleCount += 1
        puts("\nRunning cycle #{cycleCount}")
        res = targetGraph.process
        puts("\tCycle #{cycleCount} resulted in \"#{res}\"")
        case res
        when :direct
        when :implicit
        else
            break
        end
    end
end
