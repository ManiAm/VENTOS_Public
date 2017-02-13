#ifndef FIND_MODULE_H
#define FIND_MODULE_H

#include <omnetpp.h>

/**
 * @brief Provides method templates to find omnet modules.
 *
 * @ingroup baseUtils
 * @ingroup utils
 */
template<typename T = omnetpp::cModule *const>
class FindModule
{
public:
    /**
     * @brief Returns a pointer to a sub module of the passed module with
     * the type of this template.
     *
     * Returns NULL if no matching submodule could be found.
     */
    static T findSubModule(const omnetpp::cModule *const top)
    {
        for (omnetpp::cModule::SubmoduleIterator i(top); !i.end(); i++)
        {
            omnetpp::cModule *const sub = *i;
            // this allows also a return type of read only pointer: const cModule *const
            T dCastRet = dynamic_cast<T>(sub);
            if (dCastRet != NULL)
                return dCastRet;
            // this allows also a return type of read only pointer: const cModule *const
            T recFnd = findSubModule(sub);
            if (recFnd != NULL)
                return recFnd;
        }

        return NULL;
    }

    /**
     * @brief Returns a pointer to the module with the type of this
     * template.
     *
     * Returns NULL if no module of this type could be found.
     */
    static T findGlobalModule() {return findSubModule(omnetpp::getSimulation()->getSystemModule());}

    /**
     * @brief Returns a pointer to the host module of the passed module.
     *
     * Assumes that every host module is a direct sub module of the
     * simulation.
     */
    static omnetpp::cModule *const findHost(omnetpp::cModule *const m)
    {
        omnetpp::cModule* parent = m != NULL ? m->getParentModule() : NULL;
        omnetpp::cModule* node   = m;

        // all nodes should be a sub module of the simulation which has no parent module!!!
        while( parent != NULL && parent->getParentModule() != NULL  )
        {
            node   = parent;
            parent = node->getParentModule();
        }

        return node;
    }

    // the constness version
    static const omnetpp::cModule *const findHost(const omnetpp::cModule *const m)
    {
        const omnetpp::cModule* parent = m != NULL ? m->getParentModule() : NULL;
        const omnetpp::cModule* node   = m;

        // all nodes should be a sub module of the simulation which has no parent module!!!
        while( parent != NULL && parent->getParentModule() != NULL  )
        {
            node   = parent;
            parent = node->getParentModule();
        }

        return node;
    }
};

/**
 * @brief Finds and returns the pointer to a module of type T.
 * Uses FindModule<>::findSubModule(), FindModule<>::findHost(). See usage e.g. at ChannelAccess.
 */
template<typename T = omnetpp::cModule>
class AccessModuleWrap
{
public:
    typedef T wrapType;

private:
    T* pModule;

public:
    AccessModuleWrap() : pModule(NULL) { }

    T *const get(omnetpp::cModule *const from = NULL)
    {
        if (!pModule)
        {
            pModule = FindModule<T*>::findSubModule( FindModule<>::findHost(from != NULL ? from : omnetpp::getSimulation()->getContextModule()) );
        }

        return pModule;
    }
};

#endif
